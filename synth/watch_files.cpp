#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/inotify.h>

#include "watch_files.h"
#include "util.h"
#include "globals.h"
#include "in_synth_cli.h"

using namespace std;

int fd=-1;
map<int, set<int> > inotify_map;
pthread_mutex_t inotify_map_mutex;

void watch_files_cleanup(void* unused)
{
	if (fd==-1)
	{
		output_verbose("NOTE: no cleaning necessary for watch-files-thread");
	}
	else
	{
		output_verbose("NOTE: cleaning up for watch-files-thread...");
		
	}
}

void* watch_files(void* unused)
{
	pthread_cleanup_push(watch_files_cleanup, NULL);
	
	pthread_mutex_init(&inotify_map_mutex, NULL);
	
	fd=inotify_init();
	if (fd==-1)
	{
		output_warning("WARNING: could not initalize inotify. you must inform me about\n"
									 "         updated files manually.");
		while (true) sleep(10);
	}
	else
	{
		for (int i=0;i<128;i++) // add watches for all loaded programs
			if (programfile[i]!="")
				add_watch(i);
		
		inotify_event ev;
		size_t s;
		while (true)
		{
			s=read (fd, &ev, sizeof(inotify_event));
			while (s<sizeof(inotify_event))
				s+=read (fd,(char*)&ev + s, sizeof(inotify_event)-s);
			
			pthread_mutex_lock(&inotify_map_mutex);
			
			if (ev.mask & IN_MODIFY)
			{
				if (verbose)
				{
					string str="";
					set<int>& tmp=inotify_map[ev.wd];
					for (set<int>::iterator it=tmp.begin(); it!=tmp.end(); it++)
						str+="#"+IntToStr(*it)+" ";
					
					output_verbose("NOTE: reloading programs "+str+"...");
				}

				set<int>& tmp=inotify_map[ev.wd];
				for (set<int>::iterator it=tmp.begin(); it!=tmp.end(); it++)
					lock_and_load_program_no_watch_updates(*it, programfile[*it]);
			}
			else if (ev.mask & (IN_MOVE_SELF | IN_DELETE_SELF))
			{
				if (verbose)
				{
					string str="";
					set<int>& tmp=inotify_map[ev.wd];
					for (set<int>::iterator it=tmp.begin(); it!=tmp.end(); it++)
						str+="#"+IntToStr(*it)+" ";
					
					output_verbose("NOTE: removed watch for programs "+str);
				}
				
				inotify_map.erase(ev.wd);
				inotify_rm_watch(fd,ev.wd);
			}
			else if (ev.mask != IN_IGNORED)
			{
				output_note("NOTE: in watch_files-thread: unknown event received ("+IntToStrHex(ev.mask)+")");
			}

			pthread_mutex_unlock(&inotify_map_mutex);
		}
	}
		
	pthread_cleanup_pop(0);
}

void remove_watch(int prog)
{
	if (watchfiles)
	{
		map<int, set<int> >::iterator mit;
		set<int>* tmp;
		set<int>::iterator sit;
		
		pthread_mutex_lock(&inotify_map_mutex);
		
		//search in all known watch descriptors
		for (mit=inotify_map.begin(); mit!=inotify_map.end(); mit++)
		{
			tmp=&(mit->second);
			sit=tmp->find(prog);
			
			//search for some wd which affects $prog
			if (sit!=tmp->end()) //found?
			{
				//erase $prog from the affect-set
				tmp->erase(sit);
				if (tmp->empty())
				{
					//if the affect-set is now empty, we can garbage-collect
					//the wd (i.e., remove it)
					cout << "garbage collecting wd #"<<mit->first<<endl;
					inotify_rm_watch(fd, mit->first);
					inotify_map.erase(mit);
				}
				
				//we're done now
				break;
			}
		}
		
		pthread_mutex_unlock(&inotify_map_mutex);
	}
}

void add_watch(int prog)
{
	if (watchfiles)
	{
		int wd=inotify_add_watch(fd, programfile[prog].c_str(), IN_MODIFY | IN_MOVE_SELF | IN_DELETE_SELF);
		
		pthread_mutex_lock(&inotify_map_mutex);
		
		if (wd!=-1)
		{
			inotify_map[wd].insert(prog);
		}
		else
		{
			//TODO: warning
		}
		
		pthread_mutex_unlock(&inotify_map_mutex);
	}
}

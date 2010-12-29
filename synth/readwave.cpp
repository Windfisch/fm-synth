#include <string>
#include <cstring>

#include "readwave.h"
#include "util.h"
#include "fixed.h"

using namespace std;


unsigned long int le_dword(unsigned char *b)
{
	return b[0]+256*b[1]+256*256*b[2]+256*256*256*b[3];
}
unsigned int le_word(unsigned char *b)
{
	return b[0]+256*b[1];
}

signed int le_sword(unsigned char *b)
{
	int x=le_word(b);
	if (x & (1<<15) )
		return - ((~(x-1))&0xFFFF);
	else
		return x;
}

void safe_fread(void* buf, int size, int n, FILE* f)
{
	int x=fread(buf,size,n,f);
	
	if (x!=n)
		throw string("got end-of-file or error while reading from file");
}

void read_wave(const char *fn, custom_wave_t *result)
{
	try
	{
		int fmt=0, chans=0, sr=0, bits=0, bytes=0, filelen=0;
		unsigned char buf[100];

		FILE *f=fopen(fn,"r");
		if (f==NULL)
			throw string("could not open file");
		
		safe_fread(buf, 1, 12, f);
		
		if ((memcmp(buf,"RIFF",4)==0) && (memcmp(buf+8,"WAVE",4)==0))
		{
			filelen=le_dword(buf+4);
			
			while (!feof(f))
			{
				int chunklen;
				
				safe_fread(buf,1,8,f); //read chunk name and chunk size
				
				if (memcmp(buf,"fmt ",4)==0) //it's the fmt-chunk!
				{
					chunklen=le_dword(buf+4);
					safe_fread(buf,1,chunklen,f);
					
					fmt=le_word(buf); //should be 1 for PCM
					chans=le_word(buf+2); //number of channels
					sr=le_dword(buf+4); //sampling rate
					bits=le_word(buf+14); //bits per sample (8 or 16)
					
					if (fmt!=1)
						throw string("invalid format, expected PCM");
					
					if ((bits!=8) && (bits!=16))
						throw string("invalid format, expected 8 or 16 bits");
					
					if (chans==0)
						throw string("invalid format, n_channels may not be zero");
					
					if (chans>=2)
						output_note("NOTE: wavefile '"+string(fn)+"' is multichannel, using the left\nchannel and ignoring the rest...");			
					
					if (sr==0)
						throw string("sampling rate may not be zero");
						
				}
				else if (memcmp(buf,"data",4)==0) //it's the data-chunk!
				{
					chunklen=le_dword(buf+4);
					
					if (sr==0)
						throw string("found data chunk before the fmt chunk");
					
					if (bits==8)
						bytes=1;
					else if (bits==16)
						bytes=2;

					int n_samples=chunklen/(bytes * chans);
					
					result->samp_rate=fixed_t(sr)<<SCALE;
					result->wave_len=n_samples;
					result->wave=new fixed_t[n_samples];
					
					double sample;
					for (int i=0;i<n_samples;i++)
					{
						if (feof(f))
							throw string("unexpected end-of-file");
							
						safe_fread(buf,1,bytes*chans,f);
						if (bits==8)
							sample=(buf[0]-128)/128.0;
						else
							sample=le_sword(buf)/double(1<<15);
						
						result->wave[i]=sample*ONE;
					}
					break;
				}
				else //unknown chunk, skip it
				{
					chunklen=le_dword(buf+4);
					safe_fread(buf,1,chunklen,f);
				}
			}
		}
		else
		{
			//not a valid wave file!
			throw string("not a valid RIFF-WAVE file");
		}
	}
	catch (string err)
	{
		output_warning("ERROR: could not read '"+string(fn)+"': "+err+"\n  a default waveform (sine) will be loaded");
	}
}

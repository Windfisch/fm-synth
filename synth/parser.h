#ifndef __PARSER_H__
#define __PARSET_H__


#include <set>
#include <map>
#include <list>
#include <string>

#include "fixed.h"
#include "programs.h"

using namespace std;

class Parser
{
	public:
		Parser();
		~Parser();
		void parse(string fn);
		program_t get_results() const;
		
	private:
		void init_stuff();
		void uninit_stuff();
		static string extract_array_name(string s);
		static list<string> extract_terms(string s);
		static list<string> extract_factors(string s); 
		static list<term_t> extract_formula(string s);
		static param_factor_t parse_pfactor(string s);
		static int extract_array_index(string s, int dim);
		
		int n_osc;
		oscillator_t *osc;
		env_settings_t *env;
		set<parameter_t> affect[128];
		map< parameter_t, list<term_t> > formula;
		int controller_default[128];
		filter_params_t filter;
		
		pfactor_formula_t pfactor;
		
		fixed_t sync_factor;
};


#endif

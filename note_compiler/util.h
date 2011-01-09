#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include "programs.h"

using namespace std;

string IntToStr(int i);
string IntToStrHex(int i);

bool isnum(string s);
bool isfloat(string s);

string remove_all_spaces(string s);
string trim_spaces(string s);

void output_warning(string s);
void output_note(string s);
void output_verbose(string s);

bool param_needs_index(parameter_enum p);
parameter_enum param_to_enum(string param);

string extract_var(string s);
string extract_val(string s);

string fileext(string f);


#endif

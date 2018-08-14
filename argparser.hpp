#ifndef ARGPARSER_HPP_
#define ARGPARSER_HPP_

#include <iostream>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <regex>
#include <iterator>
#include <sstream>
#include <type_traits>
#include <stdexcept>
#include <unordered_map>

#ifndef ARG_PARSE
#define ARG_PARSE(app, argc, argv)  try{(app).parse((argc), (argv));} catch(std::runtime_error &e){ std::cout << e.what(); return 0; }
#endif

namespace cli {

class Option;

using OptResult = std::vector<std::string>;
using OptCallback = std::function<int(OptResult, bool)>;
using SharedOption = std::shared_ptr<Option>;

/*--------------------------------------------------------*/
//utility functions
template <typename T> struct is_std_basic_string : std::false_type{};
template <typename... ArgsT> struct is_std_basic_string <std::basic_string<ArgsT...>> : std::true_type {};
template <typename T> constexpr bool is_std_basic_string_v = is_std_basic_string<T>::value;

// std::string  => basic_string<char, char_traits<char>, std::allocator>
// std::wstring => basic_string<wchar, ...>
inline bool valid_first_char(char c) { return std::isalpha(c, std::locale()) || c == '_';};

// Function: check_prefix
// return true if the give string is a valid option key (start with - or --)
inline bool check_prefix(const std::string &current){
  if(current.size() > 2 && current.substr(0,2) == "--" && valid_first_char(current[2]))
    return true;
  else if(current.size() > 1 && current[0] == '-' && valid_first_char(current[1]))
    return true;
  else
    return false;
}

// Function: split_option
// the function split a given string to option key and value 
// --d=1   => name: --d    value: 1
// -d      => name: -d     value: 
// -data=1 => name: -data  value: 1
inline bool split_option(const std::string &current, std::string &name, std::string &value){
  if(auto loc=current.find("="); loc != std::string::npos){
    name = current.substr(0, loc);
    value = current.substr(loc+1);
  }
  else{
    name = current;
    value = "";
  }
  return check_prefix(current);
}

template<typename T>
int assign_single(OptResult &res, int idx, T& arg){

  using U = std::decay_t<T>;
  std::string str = res[idx];
  if constexpr(is_std_basic_string_v<U>){
    arg = str;
    return 0;
  }

  std::istringstream iss (str);
  iss >> arg;
  if(iss.fail() == 1 || iss.bad() == 1)
    return -(idx+1);
  return 0;

}

// Function: assign_result
// assign the result from res to the argument pack
// Return zero on success
// Return positive value N when res.size() < sizeof...(args)
// Return negative value N when the |N|-th arg on conversion 
template<typename... Args>
int assign_result(OptResult &res, Args&&... args){

  if(sizeof...(args) > res.size()) {
    return res.size()+1;
  }
  
  // TODO: verify this...
  int idx = 0;
  int min = 0;
  (..., (min = std::min(min, assign_single(res, idx++, args))));
  
  return min;

  //int idx=0;
  //std::vector<int> ret_vals;
  //(..., ret_vals.push_back(assign_single(res, idx++, args)));
  //int ret_val = 0;
  //for(const int& r: ret_vals)
  //  if(r < ret_val)
  //    ret_val = r;
  //return ret_val;
}

// Procedure: get_names
// extract the option name(s) from a given string and store them in the vector
// -d,-df,-ff -ds    => -d -df -ff -ds
inline void get_names(const std::string &name, std::vector<std::string> &names){

  names.clear(); //make sure the output vector is empty

  // TODO: use unordered_set ...
  //std::unordered_set<std::string> name_set;

  std::unordered_map<std::string, bool> name_map;

  const std::regex _delimiter{"\\s+|,+|;+"};
  std::sregex_token_iterator pos(name.cbegin(), name.cend(), _delimiter, -1);
  std::sregex_token_iterator end;
 
  for(; pos != end; pos++){
    std::string tmp;
    std::string opt_name;  
    std::string opt = pos->str();
    if(split_option(opt, opt_name, tmp)){
      if(name_map.find(opt_name) != name_map.end())//repeated name
        continue;
      else{
        name_map[opt_name] = true; 
        names.push_back(opt_name);
      }
    }
    else {
      continue;
    }
  }
}

/*--------------------------------------------------------*/

class Option{

  private:
    
    bool _callback_run{false};
    bool _parsed{false};

    std::vector<std::string> _names;
    std::string _description;

    OptResult _results; 
    OptCallback _callback;
    
    // TODO: static variable initialization order problem

  public:
    
    template <typename C>
    Option(const std::string &name, C&& callback, const std::string &description)
      : _description(description),
        _callback(std::forward<C>(callback)) {
      get_names(name, _names); 
      _results.clear(); 
    }
    
    void parsed(){ _parsed = true; }

    bool is_parsed(){ return _parsed;  }
    
    Option *add_result(const std::string &s){
      _results.push_back(std::move(s));
      _callback_run = false;
      return this;
    }

    void run_callback(){

      if(_callback_run || !_parsed) return;

      int local_result;
      local_result = _callback(_results, _parsed);
      _callback_run = true;
     
      if(local_result < 0){
        std::string err_msg = "conversion failure on option" + option_names() + " on argument " + std::to_string(-local_result) + "\n";
        throw std::runtime_error(err_msg);
      }
      else if(local_result > 0){ 
        std::string err_msg = "option" + option_names() + " is not assigned properly starting from argument " + std::to_string(local_result) + "\n";
        throw std::runtime_error(err_msg);
      }
    }  

    std::string option_names(){
      std::string sum;
      for(const std::string &n: _names){
        sum = sum + " " + n;
      }
      return sum;
    }  
 
    std::string description(){ return _description; }
 
    void print_info() const{
      
      for(const std::string &name: _names)
        std::cout << name << " ";
      std::cout << "\t\"" << _description << "\"" << std::endl;
  
    }


};

class ArgParser {

  public:

    struct OptionHandle {
      public:
        OptionHandle(SharedOption opt_ptr) : ptr(opt_ptr.get()) {}
        OptionHandle(Option *opt_ptr) : ptr(opt_ptr) {}
      private:
        Option *ptr;
    };


  private:

    std::string _name;
    std::unordered_map<std::string, SharedOption> _options; 

    bool _parsed{false};

    template <typename C> 
    Option *_add_option(const std::string &name, C&& callback, const std::string &description=""){
     
      std::vector<std::string> names;
      get_names(name, names);     
 
      if(std::find_if(std::begin(names), std::end(names), [this](const std::string &n){ return this->_options.find(n) != this->_options.end(); }) 
        == std::end(names)){
        SharedOption op = std::make_shared<Option>(name, callback, description);
        for(const std::string &n: names)
          _options[n] = op;
        return op.get();
      }
      else{
        throw std::runtime_error("option already added\n");
      }

    }
    
    // backtrace the args until reaching a single option
    void _parse_single(std::vector<std::string> &args){

      std::string current = args.back();
      std::string name;
      std::string value;

      if(!split_option(current, name, value)){
        std::string err_msg = "argument " + current + " is not an option or flag\n";
        throw std::runtime_error(err_msg);
      }         

      auto op_ite = _options.find(name);

      if(op_ite == _options.end()){
        args.pop_back();
        std::string err_msg = "unidentified option: " + name + " use single flag --help for help\n";
        throw std::runtime_error(err_msg);
      }

      args.pop_back();

      if(value.size() > 0) // "=" an argument
        args.push_back(value);
      
      SharedOption op = op_ite->second;
      
      if(op->is_parsed()){
        std::string err_msg = "option: " + current + " is parsed more than once\n";
        throw std::runtime_error(err_msg);
      }
      op->parsed(); // mark the option as parsed

      while(!args.empty() && !check_prefix(args.back())){
        op->add_result(args.back());
        args.pop_back();  
      }

    }

    
    //has to be reverse order
    void _parse(std::vector<std::string> &args){
      
      // TODO: exit on help std::exit(EXIT_SUCCESS)
      if(args.size() == 1 && args[0]=="--help"){
        print_options();
        return;
      }
 
      if(_parsed)
        return; //not reuseable for now
 
      while(!args.empty()){
        _parse_single(args);
      }

      for(const auto &op_ite: _options){
        op_ite.second->run_callback();
      }      

    }
  
  public:

    ArgParser(){ _name=""; }    
  
    ArgParser(const std::string &name): _name(name) {}
 
    template <typename... Args>
    OptionHandle add_option(const std::string &name, const std::string &description, Args&&... args){
     
      OptCallback func = [&args...](OptResult res, bool parsed){
        return assign_result(res, args...); //test for fold expression
      }; 
  
      Option *opt = _add_option(
        name, 
        func, 
        description
      );

      return OptionHandle(opt);
    }

  
    OptionHandle add_flag(const std::string &name, const std::string &description, bool &flag){

      OptCallback func = [&flag](OptResult res, bool parsed){
        flag = parsed;
        return 0;
      };

      Option *opt = _add_option(
        name, 
        func, 
        description
      );

      return OptionHandle(opt);

    }

    void parse(const std::vector<std::string> &args){
      std::vector<std::string> rargs = args;
      std::reverse(rargs.begin(), rargs.end());
      _parse(rargs);
    }
    
    void parse(int argc, const char *const *argv){

      if(_name.empty())
        _name = argv[0];

      std::vector<std::string> args; 
 
      for(int i = argc-1; i>0; i--)
        args.emplace_back(argv[i]);
   
      _parse(args); 
      _parsed = true; 
    }

    void print_options(){
      std::cout << "Option:\t\t\tDescription:" << std::endl;
      for(const auto &op : _options)
        std::cout << op.first << "\t\t\t" << op.second->description() << std::endl; 
    }

};

}; // end of namespace argparser.


#endif

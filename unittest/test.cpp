#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <argparser.hpp>

#include <string>
#include <vector>
#include <algorithm>

TEST_CASE("ArgParser.Option"){

  std::string opt1, opt2;
  
  cli::ArgParser app;
  auto op = app.add_option("-o, --option", "Specify two arguments with --option or -o", opt1, opt2);
  
  SUBCASE("CaseSensitive"){ 
    std::vector<std::string> args {"--option", "a", "bc"};
    app.parse(args);
    CHECK(opt1 == "a");
    CHECK(opt2 == "bc");
  }
 
}

TEST_CASE("ArgParser.Flag"){

  bool flag = false;
  cli::ArgParser app;
  auto op = app.add_flag("--flag,  -f", "Specify a flag", flag);
  
  SUBCASE("CaseSensitive"){
    std::vector<std::string> args{"--flag"};
    app.parse(args);
    CHECK(flag == true);
  }

}

TEST_CASE("ArgParser.MultiOpt"){

  std::vector<std::string> args {"-opt1=opt1file1", "opt1file2", "--flag", "--option2", "opt2file1", "opt2file2", "opt2file3", "a", "b" , "--option3", "opt3file1"};

  cli::ArgParser app;
  bool flag = false;
  std::string opt1[2];
  std::string opt2[3];
  std::string opt3;

  app.add_option("--option1, -opt1", "Specify arguments with option1", opt1[0], opt1[1]);
  app.add_option("--option2", "Specify arguments with option2", opt2[0], opt2[1], opt2[2]);
  app.add_option("--option3", "Specify arguments with option3", opt3);
  app.add_flag("--flag", "Specify the flag", flag);

  app.parse(args);

  CHECK(opt1[0] == "opt1file1");
  CHECK(opt1[1] == "opt1file2");

  CHECK(opt2[0] == "opt2file1");
  CHECK(opt2[1] == "opt2file2");
  CHECK(opt2[2] == "opt2file3");

  CHECK(opt3 == "opt3file1");

  CHECK(flag == true);

}

TEST_CASE("ArgParser.Failures"){

  cli::ArgParser app;
  int a=0;
  float b=0.0f;
  
  app.add_option("--number, -n", "Specify two numbers", a, b);

  SUBCASE("FirstArgConversion"){
    std::vector<std::string> args{"-n", "hello", "12.34"}; 
    try{ app.parse(args); }
    catch(std::runtime_error &e){
      std::cout << "Caught exception: "<< e.what();
    }
  }

  SUBCASE("SecondArgConversion"){
    std::vector<std::string> args{"-n", "12", "hello"};
    try{ app.parse(args); }
    catch(std::runtime_error &e){
      std::cout << "Caught exception: "<< e.what();
    }
  }

  SUBCASE("SecondArgMissing"){
    std::vector<std::string> args{"-n", "12"};
    try{ app.parse(args); }
    catch(std::runtime_error &e){
      std::cout << "Caught exception: "<< e.what();
    }
  }

  SUBCASE("NoLeadingOption"){
    std::vector<std::string> args{"hello", "-n", "1", "2"};
    try{ app.parse(args); }
    catch(std::runtime_error &e){
      std::cout << "Caught exception: "<< e.what();
    }
  }

  SUBCASE("SameOptionName"){
    try{
      app.add_option("--num, --number", "Specify numbers", a, b);
    }
    catch(std::runtime_error &e){
      std::cout << "Caught exception: " << e.what();
    }
  }

}

// partial arguments
TEST_CASE("ArgParser.PartialOptions"){

  cli::ArgParser app;
  int opt1[2];
  int opt2[3];
  float opt3;

  app.add_option("-opt1, --option1", "Specify option 1", opt1[0], opt1[1]);
  app.add_option("-opt2, --option2", "Specify option 2", opt2[0], opt2[1], opt2[2]); 
  app.add_option("-opt3, --option3", "Specify option 3", opt3); 

  SUBCASE("NormalCase"){
    std::vector<std::string> args{"-opt3", "10", "-opt2=9", "8", "7", "-opt1","6","5"};
    app.parse(args);
    CHECK(opt1[0] == 6);
    CHECK(opt1[1] == 5);
    CHECK(opt2[0] == 9);
    CHECK(opt2[1] == 8);
    CHECK(opt2[2] == 7);
    CHECK(opt3 == 10);
  }

  SUBCASE("IgnoreOpt2"){
    std::vector<std::string> args{"-opt3", "10", "-opt1","6","5"};
    app.parse(args);
    CHECK(opt1[0] == 6);
    CHECK(opt1[1] == 5);
    CHECK(opt3 == 10);
  }

  SUBCASE("MissingOpt2"){
    std::vector<std::string> args{"-opt3", "10", "-opt2", "-opt1","6","5"};
    try{ app.parse(args); }
    catch(std::runtime_error& e){
      std::cout << "Caught exception: " << e.what();
    }
  }

}



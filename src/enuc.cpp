#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <ctype.h>
#include "pugixml.hpp"

int elemZ[114]={0};
std::string elemSymbol[114]={""};

bool load_elements(){
  std::ifstream elem_file("elements.txt");
  std::string str1, str2;
  int x;

  for(int i=0; i<114; i++){
    elem_file >> str1 >> str2 >> x;


    //webnucleo databse uses only lower case symbols for chemical elements
    std::locale loc;
    std::string str2_low="";
    for (std::string::size_type i=0; i<str2.length(); ++i)
      str2_low.push_back( std::tolower(str2[i],loc) );

    elemSymbol[i]=str2_low;

    elemZ[i]=x;

    if(elem_file.fail()) return false;
    //std::cout << elemZ[i] <<": "<<elemSymbol[i]<<std::endl;
  }

return true;
}

bool is_digits(const std::string &str){
  return str.find_first_not_of("0123456789") == std::string::npos;
}

bool get_A(const std::string & mass, int & A){
  A=-1;
  std::stringstream stream(mass);
  stream >> A;
  if(!stream) return false;
  else return true;
}

bool get_element(char ch1, char ch2, std::string &element, int &Z){
  element = ""; Z= -1;
  if(isupper(ch1)){
    element.push_back(ch1);
    if(islower(ch2)) element.push_back(ch2);

    for(int i=0; i<113; i++){
      if(element == elemSymbol[i]){
        Z= elemZ[i];
        break;
      }
    }
    return true;
  }
  else return false;
}


//check if this is an interesting isotope; e.g., is it neutron-rich enough?
bool is_interesting(int Z,int A, int Zmin=26, int delta=2){

  double p0 = -4.801;
  double p1 = 1.416;
  double p2 = 2.372e-3;

  double Nstab; 
  Nstab= p0 + p1*Z + p2*Z*Z;

  if( Z>=Zmin && ((A-Z)>(Nstab+delta)) ){
    //hard coded limit for Sn, Te, Xe isotopes that deviate from parameterization
    if(Z==50){ if( A>127 ) return true;}
    else if(Z==52){ if( A>131 ) return true;}
    else if(Z==54){ if( A>137 ) return true;}

    else return true;
  }
  return false;
}


int main()
{
  using namespace pugi;

  // Load XML file from fstream

  std::ifstream xml_file("outHCNO1_14n.xml");

  std::ifstream flows_file("flows1.txt");

  if(!load_elements()){
    std::cerr << "ERROR: can't load file with info on chemical elements" << std::endl;
    return 1;
  }


  if(!xml_file)
    {
      std::cerr << "ERROR: opening XML file: " << std::endl;
      return 1;
    }


  xml_document doc;
  xml_parse_result res = doc.load(xml_file);

  if(!res) {
      std::cerr << "ERROR: " << res.description() << std::endl;
      char * buffer = new char[15];
      xml_file.seekg(res.offset);
      xml_file.read(buffer,15);

      std::cerr << "Error offset: " << res.offset << " (error at [..." << buffer << "]\n\n";
      delete[] buffer;
      return 1;
    }

  std::cout << "load? " << res.description() << std::endl;

  std::cout << "ejemplo" << doc.child("nuclear_network").child("nuclear_data").first_child().child("z").value() << std::endl;

  int Nisotopes=0;
  int Ngood= 0;

  xml_node nuclear_data = doc.child("libnucnet_input").child("nuclear_network").child("nuclear_data");

  for(  xml_node nuclide = nuclear_data.child("nuclide"); nuclide; nuclide = nuclide.next_sibling("nuclide")){


    Nisotopes++; //is_rproc= false;

    int A, Z;
    double ME;

    //    xml_node a = nuclide.child("a");
    Z  = nuclide.child("z").text().as_int();
    A  = nuclide.child("a").text().as_int();
    ME = nuclide.child("mass_excess").text().as_double();

    if(Z>=0 && Z<114){
      std::cout << A<< elemSymbol[Z] << " ("<<Z<<"): " << ME <<" MeV/c2 " << std::endl;
      Ngood++;
    }
    else std::cout << "who the heck am I!!!??? " << Z << " " << A << std::endl;

    //TO-DO:
    //add A, element symbol, and ME in vectors, which will then be searched when reading each reaction
    //other option: create a 2D vector of coordinates A (N), Z, and search this directly? Maybe a bit faster?... but brute force should be ok in this case
    
  } //loop over nuclides

  //TO-DO
  //close input xml file to save space?
  //go through input of flows
  //find timestep
  //read isotopes for each line with reaction; split reactants and products (identify electrons, neutrinos, etc..)
  //get M_initial, M_final -> calculate Qvalue
  //approximation for neutrinos?

  std::cout << "\n\nNumber of entries in database: " << Nisotopes << std::endl;
  std::cout << "Number of isotopes I understood: " << Ngood << std::endl;


}

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <list>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/ref.hpp>


using boost::reference_wrapper;
using boost::ref;

//todo namespace

//todo template this
class ConfigNode {
  // insert private data here // 
  bool status=false;
  int size=0;
  unsigned int hash=0;

  //intrusive serialization data
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
     ar & BOOST_SERIALIZATION_NVP(loaddir);
     ar & BOOST_SERIALIZATION_NVP(ignorelist);
     ar & BOOST_SERIALIZATION_NVP(status);
     ar & BOOST_SERIALIZATION_NVP(size);
  }
   
public:
  // insert public data here
  std::string loaddir;
  std::string ignorelist;

  void init () 
  {
    loaddir="";
    ignorelist="";
    status=false;
    size=0;
    hash=0;
  }

  friend std::ostream& operator<<(std::ostream &os, const ConfigNode &confignode);
  ConfigNode () { init(); }
  virtual ~ConfigNode () {}
};

BOOST_CLASS_VERSION(ConfigNode, 1)

std::ostream& operator<<(std::ostream &os, const ConfigNode &confignode)
{
  return os << "config_node loaddir[" <<confignode.loaddir<< "/" << confignode.size << "/" << \
     confignode.status << "/"<< confignode.ignorelist << "]";
}


class Config
{
  friend class boost::serialization::access;
  std::list <ConfigNode> nodes;
  std::list <reference_wrapper<ConfigNode>> rnodes;
  std::string filename;
  ConfigNode cnnull;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    if(!rnodes.size()) 
    {
        ar & BOOST_SERIALIZATION_NVP(nodes); 
    }
    else
    {
	std::list <ConfigNode> nodes; //mask class var
	std::list<reference_wrapper<ConfigNode>>::const_iterator it;
	for (it = rnodes.begin(); it != rnodes.end(); ++it) {
	   nodes.push_back((*it).get());
    	}
        ar & BOOST_SERIALIZATION_NVP(nodes); 
    }
  }
public:
  friend std::ostream& operator<<(std::ostream &os, const Config &config);
  Config(){}
  virtual ~Config () {}

  void addConfigNode(ConfigNode &node)
  {
    rnodes.push_back(ref(node));
  }

  int getConfigNode(std::string loaddir, ConfigNode &cn)
  {
    std::list<reference_wrapper<ConfigNode>>::const_iterator it;
    for (it = rnodes.begin(); it != rnodes.end(); ++it) {
       ConfigNode &cni=(*it).get();
       if (boost::iequals(loaddir, cni.loaddir))
       {
           cn=cni;
           return 0;
       } 	
    }
    return 1;
  }

  ConfigNode& getConfigNode(std::string loaddir)
  {
    std::list<reference_wrapper<ConfigNode>>::const_iterator it;
    for (it = rnodes.begin(); it != rnodes.end(); ++it) {
       ConfigNode &cni=(*it).get();
       if (boost::iequals(loaddir, cni.loaddir))
           return cni;
    }
    return cnnull;
  }


  void save() { save(filename); }
  void save(std::string fname)
  {
    //unsigned int flags = boost::archive::no_header;
    filename=fname;
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);//, flags); 
    oa << BOOST_SERIALIZATION_NVP(this);
  }

  void load() { load(filename); }
  void load(std::string fname)
  {
    filename=fname;
    std::ifstream ifs(filename);
    assert(ifs.good());
    rnodes.clear();

    boost::archive::xml_iarchive xml(ifs);
    xml >> BOOST_SERIALIZATION_NVP(*this);
    std::list<ConfigNode>::const_iterator it;
    std::list<reference_wrapper<ConfigNode>>::const_iterator rit;
    
    for (it = nodes.begin(); it != nodes.end(); ++it) {
       rnodes.push_back(ref((ConfigNode &)(*it))); //need proper cast here
    }

  } 

  std::string to_string ()
  {
    std::ostringstream sout; sout << (*this);
    return sout.str();
  }


};


std::ostream& operator<<(std::ostream &os, const Config &config)
{
  std::ostringstream sout; sout << "config ";

  std::list<reference_wrapper<ConfigNode>>::const_iterator it;
  for (it = config.rnodes.begin(); it != config.rnodes.end(); ++it) {
    sout << (*it).get();
  }
  return os << sout.str();
}



bool CHK_FILE_FOR_STR(std::string filename, std::string str)
{
  std::ostringstream out;
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    out << in.rdbuf();
    in.close();
  }
  else return false; 

  return (out.str().find(str) != std::string::npos);
}

//BOOST_CLASS_IMPLEMENTATION(ConfigNode, object_serializable)
BOOST_CLASS_IMPLEMENTATION(Config, object_serializable)
/*
int main (int argc, char **argv)
{
  Config config1, config2;
  ConfigNode cn1, cn2;
  config1.addConfigNode(cn1);
  config1.addConfigNode(cn2);

  cn1.loaddir="site_heavy";
  cn2.loaddir="site_light";
  config1.save("site_config.xml");

  config2.load("site_config.xml");
  std::cout << config2 << std::endl; 
  std::cout << config1 << std::endl; 
  std::cout << CHK_FILE_FOR_STR("site_config.xml", "<loaddir>site_heavy") << std::endl; 

  ConfigNode &cn3=config2.getConfigNode("site_light");
  std::cout << cn3 << std::endl;
  cn3.ignorelist="TEST";
  config2.save();
  std::cout << CHK_FILE_FOR_STR("site_config.xml", "<ignorelist>TEST") << std::endl; 


  cn3=config2.getConfigNode("afasdsdfdsa");
  std::cout << cn3 << std::endl;   
  if ( cn3.loaddir.empty() )
    std::cout << "cn3.readdir is empty" << std::endl;

  cn1.init();
  config1.load(); //reload
  std::cout << config1 << std::endl;
  std::cout << (config1.to_string().find("/TEST")  != std::string::npos) << std::endl;

  return 0;
} */


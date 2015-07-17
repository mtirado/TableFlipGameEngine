/*
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _TMX_LOADER_H_
#define _TMX_LOADER_H_

#include <string>
#include <vector>
#include "TinyXML2/tinyxml2.h"

//TODO complete this, make a tiled game or soemthin?
using std::string;

class TMXLoader : public tinyxml2::XMLVisitor
{  
private:
    void DecodeLayer(string data, int layer);
public:  //this class isnt going to stick around long, just load, and delete.
    int tileWidth;
    int tileHeight;
    int mapWidth;
    int mapHeight;
    
    int imageWidth;
    int imageHeight;
    
    //TODO multi layers
    int numLayers;
    short *layerData;  //do not delete, hand this pointer to the resource obj
    std::vector<short *> layers; //delete these when done loading
    std::string textureFile;
    
    TMXLoader(){}
    virtual ~TMXLoader(){}
    
    virtual bool VisitEnter  (const tinyxml2::XMLDocument  &);
    virtual bool VisitExit (const tinyxml2::XMLDocument &);
    virtual bool VisitEnter (const tinyxml2::XMLElement &, const tinyxml2::XMLAttribute *);
    virtual bool VisitExit (const tinyxml2::XMLElement &);
    virtual bool Visit (const tinyxml2::XMLDeclaration &);
    virtual bool Visit (const tinyxml2::XMLText &);
    virtual bool Visit (const tinyxml2::XMLComment &);
    virtual bool Visit (const tinyxml2::XMLUnknown &);
    
    bool LoadMap(const char *filename);
};


/* extracted from base64.h 
 * René Nyffenegger rene.nyffenegger@adp-gmbh.ch
 */
#include <string>
#include <vector>

std::string base64_encode(unsigned char const* , unsigned int len);
std::vector<int> base64_decode(std::string const& s);



#endif

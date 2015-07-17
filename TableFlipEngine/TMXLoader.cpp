/*
 * contact: mtirado418@gmail.com
 *
 */ 


#include "TMXLoader.h"
#include "Utilities.h"
#include <algorithm>
using namespace tinyxml2;
using std::string;
bool TMXLoader::LoadMap(const char *filename)
{
    XMLDocument doc;
    
    //XMLError LoadFile( const char* filename );
    if (XML_NO_ERROR != doc.LoadFile(filename))
        return false;
    
    if (!doc.Accept(this))
        return false;
    
    //contiguous layers array, i dont want to be passing around std::vectors,
    numLayers = layers.size();
    layerData = new short[mapWidth * mapHeight * numLayers];
    short *curLayer;
    int writePos = 0;
    //for (int i = layers.size() - 1; i >= 0; i--)
    for (int i = 0; i < layers.size(); i++)
    {
        //back is base layer, load from bottom to top
        curLayer = layers[i];
        memcpy(&layerData[writePos++ * (mapWidth * mapHeight) ], curLayer, mapWidth * mapHeight * sizeof(short));
        delete[] curLayer;
        //layers.pop_back();
    }
    layers.clear();
    return true;
}

bool TMXLoader::VisitEnter  (const tinyxml2::XMLDocument  &doc)
{
    return true; //TODO: for performance, we may not want to return true for each of these callbacks for the visitor pattern.
}

bool TMXLoader::VisitExit (const tinyxml2::XMLDocument &doc)
{
    return true;
}

bool TMXLoader::VisitEnter (const tinyxml2::XMLElement &elem, const tinyxml2::XMLAttribute *attrib)
{
    if (string("map") == elem.Value()) 
    {
       // elem.Attribute("width", &mapWidth);
       // mapHeight = elem.Attribute("height");
       mapWidth = elem.FindAttribute("width")->IntValue();
       mapHeight = elem.FindAttribute("height")->IntValue();
    }
    else if (string("tileset") == elem.Value()) 
    {
       // elem.Attribute("tilewidth", & tileWidth);
       // elem.Attribute("tileheight", &tileHeight);
       tileWidth = elem.FindAttribute("tilewidth")->IntValue();
       tileHeight = elem.FindAttribute("tileheight")->IntValue();
    }
    else if (string("image") == elem.Value()) 
    {
        textureFile = (const char*)elem.Attribute("source");  
        imageWidth = elem.FindAttribute("width")->IntValue();
        imageHeight = elem.FindAttribute("height")->IntValue();
    }
    else if (string("layer") == elem.Value()) 
    {
        // NOTE assumes layers are same dimensions as mapHeight
        //TODO track what layer we are loading, and assign a z-value
        //elem.Attribute("name");
        //elem.Attribute("width");
        //elem.Attribute("height");
    }
    else if (string("data") == elem.Value()) 
    {
        const char* text = elem.GetText();
        DecodeLayer(text, 0);
    }

    return true;
}

void TMXLoader::DecodeLayer ( string data, int layer )
{
    //remove formatting
    data.erase (std::remove(data.begin(), data.end(), '\n'), data.end());
    data.erase (std::remove(data.begin(), data.end(), ' '), data.end());
    
    //TODO gzip decompress
    std::vector<int> b64 = base64_decode(data);
    //not deleted here, hand over to spriteresource
    layerData = new short[mapWidth * mapHeight]; //short to save space obviously, may be truncated if map is obscenely large
    int idx = 0;
    //convert base 64 value to a short
    for (unsigned int i = 0; i < b64.size(); i += 4)
    {
        int a = b64[i];
        int b = b64[i+1];
        int c = b64[i+2];
        int d = b64[i+3];
        int gid = a | b << 8 | c << 16 | d << 24;
        layerData[idx++] = (short)gid - 1; //make it 0 based.
    }
    layers.push_back(layerData);  //back is base layer
}


bool TMXLoader::VisitExit (const tinyxml2::XMLElement &elem)
{
    return true;
}

bool TMXLoader::Visit (const tinyxml2::XMLDeclaration &dec)
{
    return true;
}

bool TMXLoader::Visit (const tinyxml2::XMLText &text)
{
    return true;
}

bool TMXLoader::Visit (const tinyxml2::XMLComment &comment)
{
    return true;
}

bool TMXLoader::Visit (const tinyxml2::XMLUnknown &unknown)
{
    return true;
}


/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include <iostream>
#include <string>

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::vector<int> base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  //std::string ret;

  std::vector<int> ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        //ret += char_array_3[i];
        ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]); //  += char_array_3[j];
  }

  return ret;
}
//end base64.cpp

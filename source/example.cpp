// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include "tinyply.h"

using namespace tinyply;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
std::chrono::high_resolution_clock c;

inline std::chrono::time_point<std::chrono::high_resolution_clock> now()
{
    return c.now();
}

inline double difference_micros(timepoint start, timepoint end)
{
    return (double) std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

std::vector<uint8_t> read_file_binary(std::string pathToFile)
{
    FILE * f = fopen(pathToFile.c_str(), "rb");
    
    if (!f)
        throw std::runtime_error("file not found");
    
    fseek(f, 0, SEEK_END);
    size_t lengthInBytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::vector<uint8_t> fileBuffer(lengthInBytes);
    
    size_t elementsRead = fread(fileBuffer.data(), 1, lengthInBytes, f);
    
    if (elementsRead == 0 || fileBuffer.size() < 64)
        throw std::runtime_error("error reading file or file too small");

    fclose(f);
    return fileBuffer;
}

void write_ply_example(const std::string & filename)
{
    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<uint8_t> colors;
    
    std::vector<uint32_t> faces;
    std::vector<float> uvCoords;
    std::vector<uint8_t> faceColors;
    
    verts = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    norms = {10.f, 20.f, 30.f, 40.f, 50.f, 60.f};
    colors = {100, 200, 100, 200, 100, 200, 100, 200};
    
    faces = { 0, 5, 3, 2, 1, 0};
    uvCoords = { .50f, .55f, .60f, .65f, .70f, .75f, .125f, .225f, .325f, .425f, .525f, .625f};
    colors = {105, 205, 105, 205, 150, 250, 150, 250};
    
    std::ofstream outputFile(filename);
    std::ostringstream outputStream;
    
    PlyFile myFile;
    
    myFile.add_properties_to_element("vertex", {"x", "y", "z"}, verts);
    myFile.add_properties_to_element("vertex", {"nx", "ny", "nz"}, norms);
    myFile.add_properties_to_element("vertex", {"red", "green", "blue", "alpha"}, colors);
    
    myFile.add_properties_to_element("face", {"vertex_indices"}, faces, 3, PlyProperty::Type::INT8);
    myFile.add_properties_to_element("face", {"texcoord"}, uvCoords, 6, PlyProperty::Type::INT8);
    myFile.add_properties_to_element("face", {"red", "green", "blue", "alpha"}, faceColors);
    
    myFile.comments.push_back("generated by tinyply");
    myFile.write(outputStream, false);
    
    outputFile << outputStream.str();
    outputFile.close();
}

void read_ply_file(const std::string & filename)
{
    try
    {
        auto f = read_file_binary(filename);
        std::istringstream ss((char*)f.data(), std::ios::binary);

        PlyFile file(ss);

        std::vector<float> verts;
        std::vector<float> normals;
        std::vector<uint32_t> faces;

        for (auto e : file.get_elements())
        {
            std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
            for (auto p : e.properties)
            {
                std::cout << "\t property - " << p.name << std::endl;
            }
        }
        std::cout << std::endl;

        uint32_t vertexCount = file.request_properties_from_element("vertex", {"x", "y", "z"}, verts);
        //uint32_t normalCount = file.request_properties_from_element("vertex", {"nx", "ny", "nz"}, normals);
        uint32_t faceCount = file.request_properties_from_element("face", {"vertex_indices"}, faces);

        timepoint before = now();
        file.parse(ss, f);
        timepoint after = now();

        // Good place to put a breakpoint
        std::cout << "Read " << verts.size() << " vertex properties in " << difference_micros(before, after) << "μs" << std::endl;
        std::cout << "... and " << normals.size() << " normals " << std::endl;
        std::cout << "... and " << faces.size() << " faces " << std::endl;
    }
    catch (std::exception e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    write_ply_example("example.ply");
    //read_ply_file("assets/sofa.ply");
    return 0;
}

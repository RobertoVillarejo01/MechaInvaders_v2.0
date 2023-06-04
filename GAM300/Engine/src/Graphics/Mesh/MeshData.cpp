//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iostream>
//
//#include "Mesh.h"
//#pragma warning(disable : 4996)
//
//// Initialize the point ant the line segment meshes
////MeshData MeshData::Point = { {{0.0f, 0.0f, 0.0f}}, {}, {} };
////MeshData MeshData::Segment = { {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}, {}, {} };
//
///**
// * Given a string starting with 'f' it may still have multiple different formats
// * This function covers them and stores the output in "result"
// * @param line - The input string
// * @param mesh_data - The already retrieved data from the obj
// * @param result - Output parameter
// */
//void process_face_line(const std::string& line, const MeshData& temp_mesh, MeshData& result)
//{
//
//  // The format of the faces depend of the attributes we have
//
//  // Only positions
//  if (temp_mesh.mNormals.empty() && temp_mesh.mUVs.empty()) {
//    // Create a stream from the rest of the line
//    std::istringstream line_stream(line.substr(2));
//
//    // If there are only positions the format is "f pos1 pos2 pos3"
//    unsigned i1, i2, i3;
//    line_stream >> i1 >> i2 >> i3;
//
//    // Store the positions in the 'result' mesh
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i1) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i2) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i3) - 1]);
//  }
//
//  // Positions and normals
//  else if (!temp_mesh.mNormals.empty() && temp_mesh.mUVs.empty())
//  {
//    // The format is "f pos1//normal1 pos2//normal2 pos3//normal3"
//    int i1 = 0, i2 = 0, i3 = 0, n1 = 0, n2 = 0, n3 = 0;
//    sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &i1, &n1, &i2,
//      &n2, &i3, &n3);
//
//    // Store the result in the 'result' mesh
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i1) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i2) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i3) - 1]);
//
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n1) - 1]);
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n2) - 1]);
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n3) - 1]);
//  }
//
//  // Positions, uvs and normals
//  else if (!temp_mesh.mNormals.empty() && !temp_mesh.mUVs.empty())
//  {
//    // The format is "f pos1/uv1/normal1 pos2/uv2/normal2 pos3/uv3/normal3"
//    int i1 = 0, i2 = 0, i3 = 0, uv1 = 0, uv2 = 0, uv3 = 0,
//      n1 = 0, n2 = 0, n3 = 0;
//    sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
//      &i1, &uv1, &n1, &i2, &uv2, &n2, &i3, &uv3, &n3);
//
//    // Store the result in the 'result' mesh
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i1) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i2) - 1]);
//    result.mPositions.push_back(temp_mesh.mPositions[size_t(i3) - 1]);
//
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n1) - 1]);
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n2) - 1]);
//    result.mNormals.push_back(temp_mesh.mNormals[size_t(n3) - 1]);
//
//    result.mUVs.push_back(temp_mesh.mUVs[size_t(uv1) - 1]);
//    result.mUVs.push_back(temp_mesh.mUVs[size_t(uv2) - 1]);
//    result.mUVs.push_back(temp_mesh.mUVs[size_t(uv3) - 1]);
//  }
//}
//
///**
// * Given a filename extract its information and format it in a format
// * comprenhensible for the GPU
// * @param filename
// */
//MeshData LoadObj(const char* filename)
//{
//  // The resulting mesh_data
//  MeshData result{};
//
//  // Open the file and check for errors
//  std::ifstream file(filename);
//  if (!file.is_open()) {
//    std::cerr << "Unable to open file: " << filename << std::endl;
//    return result;
//  }
//
//  // Temporal buffer for each line
//  std::string line;
//
//  // Temporal containers for the different attributes
//  MeshData temp_mesh{};
//
//  // Read each line of the program
//  while (std::getline(file, line))
//  {
//    // Vertex Position
//    if (line.rfind("v ", 0) == 0) {
//      // Create a stream from the rest of the line
//      std::istringstream line_stream(line.substr(2));
//
//      // Add the new position to the vector
//      glm::vec3 new_pos;
//      line_stream >> new_pos[0] >> new_pos[1] >> new_pos[2];
//      // std::cout << "v " << new_pos[0] << " " << new_pos[1] << " " << 
//       //    new_pos[2] << std::endl;
//      temp_mesh.mPositions.push_back(std::forward<glm::vec3>(new_pos));
//    }
//
//    // Vertex Normal
//    else if (line.rfind("vn ", 0) == 0) {
//      // Create a stream from the rest of the line
//      std::istringstream line_stream(line.substr(3));
//
//      // Add the new normal to the vector
//      glm::vec3 new_normal;
//      line_stream >> new_normal[0] >> new_normal[1] >> new_normal[2];
//      //std::cout << "vn " << new_normal[0] << " " << new_normal[1] << " " << 
//        //  new_normal[2] << std::endl;
//      temp_mesh.mNormals.push_back(std::forward<glm::vec3>(new_normal));
//    }
//
//    // Vertex UVs
//    else if (line.rfind("vt ", 0) == 0) {
//      // Create a stream from the rest of the line
//      std::istringstream line_stream(line.substr(3));
//
//      // Add the new UV coords to the vector
//      glm::vec2 new_uv;
//      line_stream >> new_uv[0] >> new_uv[1];
//      //std::cout << "vt " << new_uv[0] << " " << new_uv[1] << std::endl;
//      temp_mesh.mUVs.push_back(std::forward<glm::vec2>(new_uv));
//    }
//
//    // Faces
//    else if (line.rfind("f ", 0) == 0) {
//      process_face_line(line, temp_mesh, result);
//    }
//  }
//
//  // After the mesh has been computed, return it
//  return result;
//}
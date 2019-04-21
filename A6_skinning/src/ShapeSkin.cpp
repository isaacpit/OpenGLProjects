#include <iostream>
#include <fstream>
#include <iomanip>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// #include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ShapeSkin.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;

void printMat4(glm::mat4 m) 
{
	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 4; ++j) {
			// mat[j] returns the jth column
			printf("%- 5.2f ", m[j][i]);
		}
		printf("\n");
	}
	printf("\n");
}

void printVec4(glm::vec4 v) {
	for (int i = 0; i < 4; ++i) {
		printf("%- 5.2f ", v[i]);
	}
	printf("\n");
	// cout << "v ( " << v.x << ", " << v.y << ", " << v.z << endl;
	
}

ShapeSkin::ShapeSkin() :
	prog(NULL),
	elemBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0),
	origPosBufID(0)
{
}

ShapeSkin::~ShapeSkin()
{
}

void ShapeSkin::loadMesh(const string &meshName)
{
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		origPosBuf = attrib.vertices;
		posBuf = attrib.vertices;
		norBuf = attrib.normals;
		origNorBuf = attrib.normals;
		texBuf = attrib.texcoords;
		assert(posBuf.size() == norBuf.size());
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			const tinyobj::mesh_t &mesh = shapes[s].mesh;
			size_t index_offset = 0;
			for(size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
				size_t fv = mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					// cerr << "fv: " << fv << endl;
					tinyobj::index_t idx = mesh.indices[index_offset + v];
					elemBuf.push_back(idx.vertex_index);
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void ShapeSkin::loadAttachment(const std::string &filename)
{
	// int nverts, nbones;
	ifstream in;
	in.open(filename);
	if(!in.good()) {
		cout << "Cannot read " << filename << endl;
		return;
	}
	string line;
	getline(in, line); // comment
	getline(in, line); // comment
	getline(in, line);
	stringstream ss0(line);
	ss0 >> nverts;
	ss0 >> nbones;
	assert(nverts == posBuf.size()/3);
	// origPosBuf = posBuf;
	weightBuf = std::vector<float> (nverts*nbones);
	nonZeroBoneIndicesBuf = std::vector<std::vector<float> >(nverts);
	nonZeroSkinWeightsBuf = std::vector<std::vector<float> >(nverts);
	nBoneInfluences = std::vector<float>(nverts);
	int maxBoneInfluences = 16;
	float thresh = .001;
	int idx = 0;
	while(1) {
		getline(in, line);
		if(in.eof()) {
			break;
		}
		
		stringstream ss(line);
		// weightBuf.at(idx) = std::vector<float>(nbones);
		nonZeroBoneIndicesBuf.at(idx) = std::vector<float>(maxBoneInfluences);
		nonZeroSkinWeightsBuf.at(idx) = std::vector<float>(maxBoneInfluences);
		int idx_nonZero = 0;
		for (int i = 0; i < nbones; ++i) {
			float wt;
			ss >> wt;
			weightBuf.at(idx*nbones + i) = wt;
			if (wt > thresh) {
				nonZeroSkinWeightsBuf.at(idx).at(idx_nonZero) = wt;
				nonZeroBoneIndicesBuf.at(idx).at(idx_nonZero) = i;
				idx_nonZero++;
			}
		}
		nBoneInfluences.at(idx) = idx_nonZero;
		idx++;
	}

	cerr << "BONE INDICES:" << endl;
	for (int i = 0; i < nverts; ++i) {
		cerr << "nBones: " << setw(3) << nBoneInfluences.at(i) << " | ";
		for (int j = 0; j < maxBoneInfluences; ++j) {
			cerr << setw(2) <<  nonZeroBoneIndicesBuf.at(i).at(j) << " ";
		} cerr << " | ";
		for (int j = 0; j < maxBoneInfluences; ++j) {
			printf("%3.3f ", nonZeroSkinWeightsBuf.at(i).at(j));
		} cerr << endl;
	}

	printf("weightBuf.sz: %d | weightBuf.size / 18: %d\n", weightBuf.size(), weightBuf.size() /nbones);
	// cout << "weightBuf.sz: " << weightBuf.size() << endl;
	// for (int i = 0; i < nverts; ++i) {
	// 	for (int j = 0; j < nbones; ++j) {
	// 		cout << weightBuf.at(i).at(j) << " ";
	// 	} cout << endl;
	// }
	in.close();
}

void ShapeSkin::loadSkeleton(const std::string &filename) {
	ifstream in;
	string line;
	in.open(filename);
	if(!in.good()) {
		cout << "Cannot read " << filename << endl;
		return;
	}
	getline(in, line); // comment 
	getline(in, line); // comment 
	getline(in, line); // comment 
	getline(in, line);
	stringstream ss0(line);
	ss0 >> nframes;
	ss0 >> nbones;
	printf("loading skeleton file: %s \n", filename.c_str());
	printf("(frames, bones): (%d , %d)\n", nframes, nbones);
	
	// int nBones = 18;
	vecTransforms = std::vector<std::vector<glm::mat4>>(nframes+2);
	bindPoseNoInverse = vector<glm::mat4>(nbones);
	bindPoseInverse = vector<glm::mat4>(nbones);
	int nLine = 0;
	while (!in.eof()) {
		getline(in, line);
		stringstream ss(line);
		// cout << "i: " << nLine << endl;
		// printf("%s\n", string(line).c_str());
		float qx, qy, qz, qw, px, py, pz;
		// cerr << "nLine: " << nLine << " | " << endl;
		for (int i = 0; i < nbones; ++i) {
			ss >> qx;
			ss >> qy;
			ss >> qz;
			ss >> qw;
			ss >> px;
			ss >> py;
			ss >> pz;
			glm::quat q(qw, qx, qy, qz);
			glm::vec4 p(px, py, pz, 1.0f);
			glm::mat4 E = mat4_cast(q);
			E[3] = p;
			// cerr << i << " " << endl;
			if (nLine == 0) { // bind pose
				bindPoseInverse.at(i) = glm::inverse(E); 
				bindPoseNoInverse.at(i) = E;
				// cout << "i: " << i << endl;
				// printMat4(E * bindPose.at(i));
			}
			else {
				// first bone
				if (i == 0) vecTransforms.at(nLine-1) = std::vector<glm::mat4>(nbones);
				vecTransforms.at(nLine-1).at(i) = E;
			}

			if (((nLine == 1) || (nLine == 0)) && i == 0) {
				printMat4(E);
			}
		}
		// cerr << endl;
		nLine++;
	}

	in.close();
	
}

void ShapeSkin::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	// glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	// glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	// glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	// glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// No texture info
	texBufID = 0;
	
	// Send the element array to the GPU
	glGenBuffers(1, &elemBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemBuf.size()*sizeof(unsigned int), &elemBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void ShapeSkin::drawBindPoseFrenetFrames(std::shared_ptr<MatrixStack> MV, float t, bool debug, float len) {
	
	for (int i = 0; i < nbones; ++i) {
		drawPoint(MV, bindPoseNoInverse.at(i), t, debug, len);
	}
	
	glLineWidth(1);

}


void ShapeSkin::drawAnimationFrenetFrames(std::shared_ptr<MatrixStack> MV, float t, bool debug, float len) {
	// cerr << "t: " << t << endl;
	
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);

	// glGenBuffers(1, &origPosBufID);
	// glBindBuffer(GL_ARRAY_BUFFER, origPosBufID);
	// glBufferData(GL_ARRAY_BUFFER, origPosBuf.size()*sizeof(float), &origPosBuf[0], GL_DYNAMIC_DRAW);

	float speed = 25.0f;
	int idx = fmod(t * speed, nframes);
	if (debug) {
		printf("posBuf.size(): %d | posBuf.size / 3: %d\n", posBuf.size(), posBuf.size() /3);
	}
	
	// cout << "posBuf.size(): " << posBuf.size() << endl;
	for (int i = 0; i < posBuf.size() / 3; ++i) {
		// cerr << "posBuf: " << i << endl;
		float px, py, pz, nx, ny, nz;
		px = origPosBuf.at(i * 3 + 0);
		py = origPosBuf.at(i * 3 + 1);
		pz = origPosBuf.at(i * 3 + 2);

		nx = origNorBuf.at(i * 3 + 0);
		ny = origNorBuf.at(i * 3 + 1);
		nz = origNorBuf.at(i * 3 + 2);
		glm::vec4 x_i(px, py, pz, 1.0f);
		glm::vec4 n_i(nx, ny, nz, 0.0f);
		glm::vec4 result_x(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 result_n(0.0f, 0.0f, 0.0f, 1.0f);
		// for (int j = 0; j < nbones; j++) {
			
		// 	glm::mat4 m_i = bindPoseInverse.at(j);
		// 	glm::mat4 m_0 = vecTransforms.at(idx).at(j);
		// 	float weight = weightBuf.at(i * nbones + j);
			
		// 	glm::vec4 tmp_x = (weight * (m_0 * (m_i * x_i)));
		// 	glm::vec4 tmp_n = (weight * (m_0 * (m_i * n_i)));
		// 	result_x += tmp_x;
		// 	result_n += tmp_n;
		// }

		for (int s = 0; s < nBoneInfluences.at(i); s++) {
			int j = nonZeroBoneIndicesBuf.at(i).at(s);
			float wt = nonZeroSkinWeightsBuf.at(i).at(s);
			glm::mat4 m_i = bindPoseInverse.at(j);
			glm::mat4 m_0 = vecTransforms.at(idx).at(j);
			// float weight = weightBuf.at(i * nbones + j);
			
			glm::vec4 tmp_x = (wt * (m_0 * (m_i * x_i)));
			glm::vec4 tmp_n = (wt * (m_0 * (m_i * n_i)));
			result_x += tmp_x;
			result_n += tmp_n;
		}
		result_x[3] = 1.0f;
		posBuf.at(i*3 +0) = result_x.x;
		posBuf.at(i*3 +1) = result_x.y;
		posBuf.at(i*3 +2) = result_x.z;

		norBuf.at(i*3 +0) = result_n.x;
		norBuf.at(i*3 +1) = result_n.y;
		norBuf.at(i*3 +2) = result_n.z;


	} 

	for (int i = 0; i < nbones; ++i) {
		// cout << "i: " << i << endl;
		drawPoint(MV, vecTransforms.at(idx).at(i), t, debug, len);
	}
	
	glLineWidth(1);

}

void ShapeSkin::drawPoint(std::shared_ptr<MatrixStack> MV, glm::mat4 E, float t, bool debug, float len) {
	MV->pushMatrix();
	glLineWidth(3);
	glBegin(GL_LINES);			
			
		MV->multMatrix(E);
		glm::vec4 p = E[3] ;
		glColor3f(1.0f, 0, 0);
		glVertex3f(p.x, p.y , p.z);
		glVertex3f(p.x+len, p.y, p.z);
		glColor3f(0, 1.0f, 0);
		glVertex3f(p.x, p.y , p.z);
		glVertex3f(p.x, p.y+len, p.z);
		glColor3f(0, 0, 1.0f);
		glVertex3f(p.x, p.y , p.z);
		glVertex3f(p.x, p.y, p.z+len);


	glEnd();
	MV->popMatrix();
}

void ShapeSkin::draw() const
{
	assert(prog);
	
	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// int skin_pos = prog->getAttribute("skinPos");
	// glEnableVertexAttribArray(skin_pos);
	// glBindBuffer(GL_ARRAY_BUFFER, origPosBufID);
	// glVertexAttribPointer(skin_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	int h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Draw
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glDrawElements(GL_TRIANGLES, (int)elemBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	// glDisableVertexAttribArray(skin_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "aqsis/ri/ri.h"

using namespace std;

/* Function: getCamposition
 * get the camera positions and directions/normals from the argument files (transformation matrix and camera position)
 */
void getCamPosition(char* matrixfile, char* camposfile, int numVertices, vector<RtFloat*> &camparam, vector<RtFloat*> &campos, vector<RtFloat*> &camnor)
{
	/* Read the files */
	ifstream infile1 (matrixfile);
	ifstream infile2 (camposfile);
	if(!infile1.is_open() || !infile2.is_open()) {
		printf("Cannot open camera file.");
		exit(2);
	}

	RtFloat i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16;
	RtFloat posx, posy, posz, dirx, diry, dirz;
	for(int i=0; i<numVertices; i++) {
		infile1 >> i1 >> i2 >> i3 >> i4
				>> i5 >> i6 >> i7 >> i8
				>> i9 >> i10 >> i11 >> i12
				>> i13 >> i14 >> i15 >> i16;
		infile2 >> posx >> posy >> posz >> dirx >> diry >> dirz;
		RtFloat* cam = new RtFloat[16];
		RtFloat* camp = new RtFloat[3];
		RtFloat* camn = new RtFloat[3];
		cam[0] = i1; cam[1] = i2; cam[2] = i3; cam[3] = i4;
		cam[4] = i5; cam[5] = i6; cam[6] = i7; cam[7] = i8;
		cam[8] = i9; cam[9] = i10; cam[10] = i11; cam[11] = i12;
		cam[12] = i13; cam[13] = i14; cam[14] = i15; cam[15] = i16;
		camp[0] = posx; camp[1] = posy; camp[2] = posz;
		camn[0] = -dirx; camn[1] = -diry; camn[2] = -dirz;
		camparam.push_back(cam);
		campos.push_back(camp);
		camnor.push_back(camn);
	}

	infile1.close();
	infile2.close();
}

/* Function: rendermsh (main)
 * generate renderman's files (.rib,.opt) and render image per vertex using renderman opensource software (3Delight)
 */
int main(int argc, char* argv[])
{
	if(argc != 4) {
		printf("Usage: rendermsh filename[.off] camera_position_filename[.txt] camera_matrix_filename[.txt]");
		exit(1);
	}

	/* Read file's name */
	char filename[1024];
	strcpy(filename, argv[1]);
	char offfile[1024];
	char* str = strtok(filename, "/");
	while(str != NULL) {
		memcpy(offfile, str, strlen(str)+1);
		str = strtok(NULL, "/");
	}

	ifstream infile (argv[1]);
	if(!infile.is_open()) {
		printf("Cannot open off file");
		exit(2);
	}

	/* Parse off file and create RIB, OPT, TIF files */
	char line[1024];
	infile.getline(line, 1024); // OFF
	int numVertices, numFaces, numEdges;
	infile >> numVertices >> numFaces >> numEdges;
	string ribfile = offfile;
	string rib_replace = ".rib";
	ribfile.replace(strlen(offfile)-4, rib_replace.size(), rib_replace);
	vector<string> optfiles; // Many opt and tif files per mesh (equal the number of vertices)
	vector<string> tiffiles;
	for(int i=0; i<numVertices; i++) {
		string opt = offfile;
		string tif = offfile;
		char ind[10]; itoa(i+1, ind, 10);
		string opt_replace = "-"; opt_replace.append(ind); opt_replace.append(".opt");
		string tif_replace = "-"; tif_replace.append(ind); tif_replace.append(".tif");
		opt.replace(strlen(offfile)-4, opt_replace.size(), opt_replace);
		tif.replace(strlen(offfile)-4, tif_replace.size(), tif_replace);
		string img = "img";
		optfiles.push_back(opt);
		tiffiles.push_back(img.append(tif));
	}
	
	/* Vertices */
	vector<RtFloat*> vertices;
	RtFloat px, py, pz;
	for(int i=0; i<numVertices; i++) {
		infile >> px >> py >> pz;
		RtFloat* p = new RtPoint;
		p[0] = px; p[1] = py; p[2] = pz;
		vertices.push_back(p);
	}

	/* Faces */
	vector<int*> faces;
	int nFaces;
	for(int i=0; i<numFaces; i++) {
		infile >> nFaces;
		if(nFaces == 3) {
			int ind1, ind2, ind3;
			infile >> ind1 >> ind2 >> ind3;
			int* f = new int[3];
			f[0] = ind1; f[1] = ind2; f[2] = ind3;
			faces.push_back(f);
		}
		if(nFaces == 4) { // unnecessary
			int ind1, ind2, ind3, ind4;
			infile >> ind1 >> ind2 >> ind3 >> ind4;
			int* f = new int[4];
			f[0] = ind1; f[1] = ind2; f[2] = ind3, f[3] = ind4;
			faces.push_back(f);
		}
	}
	infile.close();

	/* Camera Parameters */
	vector<RtFloat*> camparam;
	vector<RtFloat*> campos;
	vector<RtFloat*> camnor;
	getCamPosition(argv[3], argv[2], numVertices, camparam, campos, camnor);

	char* ribfile_cstr = new char[ribfile.size()+1];
	strcpy(ribfile_cstr, ribfile.c_str());

	/* Write RIB file: List the Polygon's positions and normals */
	ofstream outfile (ribfile_cstr);
	for(int i=0; i<numFaces; i++) {
		// Vertex position
		RtPoint points[3] = {*(vertices[*(faces[i])]), *(vertices[*(faces[i])]+1), *(vertices[*(faces[i])]+2),
							*(vertices[*(faces[i]+1)]), *(vertices[*(faces[i]+1)]+1), *(vertices[*(faces[i]+1)]+2),
							*(vertices[*(faces[i]+2)]), *(vertices[*(faces[i]+2)]+1), *(vertices[*(faces[i]+2)]+2)};
		// Normal of the vertex
		RtNormal normals[3] = {*(camnor[*(faces[i])]), *(camnor[*(faces[i])]+1), *(camnor[*(faces[i])]+2),
							*(camnor[*(faces[i]+1)]), *(camnor[*(faces[i]+1)]+1), *(camnor[*(faces[i]+1)]+2),
							*(camnor[*(faces[i]+2)]), *(camnor[*(faces[i]+2)]+1), *(camnor[*(faces[i]+2)]+2)};
		outfile << "Polygon \"P\" ";
		outfile << "[ ";
		for(int j=0; j<3; j++) {
			for(int k=0; k<3; k++) {
				outfile << points[j][k] << " ";
			}
		}
		outfile << "] \"N\" [ ";
		for(int j=0; j<3; j++) {
			for(int k=0; k<3; k++) {
				outfile << normals[j][k] << " ";
			}
		}
		outfile << "]" << endl;
	}
	outfile << "WorldEnd" << endl;
	outfile.close();

	/* Loop through all vertices and write to OPT files */
	int num_render = numVertices;

	for(int i=0; i<num_render; i++) {
		char* optfile_cstr = new char[optfiles[i].size()+1];
		char* tiffile_cstr = new char[tiffiles[i].size()+1];
		strcpy(optfile_cstr, optfiles[i].c_str());
		strcpy(tiffile_cstr, tiffiles[i].c_str());

		// View transformation matrix
		RtFloat matrix[4][4] = {*(camparam[i]), *(camparam[i]+1), *(camparam[i]+2), *(camparam[i]+3),
								*(camparam[i]+4), *(camparam[i]+5), *(camparam[i]+6), *(camparam[i]+7),
								*(camparam[i]+8), *(camparam[i]+9), *(camparam[i]+10), *(camparam[i]+11),
								*(camparam[i]+12), *(camparam[i]+13), *(camparam[i]+14), *(camparam[i]+15)};
		RtPoint pos = {*(campos[i]), *(campos[i]+1), *(campos[i]+2)};

		/* Write OPT file */
		ofstream outfile (optfile_cstr);
		outfile << "Display ";
		outfile << "\"" << tiffile_cstr << "\" ";
		outfile << "\"file\" \"rgb\"" << endl;
		outfile << "Projection \"perspective\"" << endl;
		outfile << "WorldBegin" << endl;
		outfile << "ConcatTransform [ ";
		for(int j=0; j<4; j++) {
			for(int k=0; k<4; k++) {
				outfile << matrix[j][k] << " ";
			}
		}
		outfile << "]" << endl;
		outfile << "Translate " << -pos[0] << " " << -pos[1] << " " << pos[2] << endl;
		outfile << "Scale 1.0 1.0 -1.0" << endl;

		delete[] optfile_cstr;
		delete[] tiffile_cstr;
	}
	delete ribfile_cstr;

	/* free memory */
	for(int i=0; i<numVertices; i++) {
		delete[] vertices[i];
		delete[] camparam[i];
		delete[] campos[i];
		delete[] camnor[i];
	}
	for(int i=0; i<numFaces; i++) {
		delete[] faces[i];
	}

	/* Render using 3Delight (RenderMan opensource) */
	string render_path = "\"C:/Program Files/3Delight/bin\"";
	string target_path = "\"C:/Neung/US/Stanford/2012-2013Junior/01_Fall/CS 231A/rendermsh/";

	string cd = "cd ";
	cd.append(render_path);
	system(cd.c_str()); // Go to 3Delight directory

	// Run in batches, to speed up
	int batch_size = 50;
	int num_batch_render = num_render / batch_size;
	int rem = num_render - batch_size * num_batch_render;

	for (int i=0; i<=num_batch_render; i++) {
		string cd = "cd ";
		string render = "renderdl -t 2 -res 50 50 ";
		cd.append(render_path);
		system(cd.c_str());

		int f = (i == num_batch_render) ? rem : batch_size;
		for(int j=0; j<f; j++) {
			render.append(target_path);
			render.append(optfiles[i*batch_size+j]);
			render.append("\" ");
			render.append(target_path);
			render.append(ribfile);
			render.append("\" ");
		}

		system(render.c_str());
	}

	return 0;
}
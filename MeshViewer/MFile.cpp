#include "MFile.h"

MFile::MFile(const char* filename) {
	char buf[256];
	char *inner, *pch;
	float xyz[3];
	int i;

	MFace tempFace;

	FILE* file = fopen(filename, "r");
	
	loaded = 0;

	if (!file) {
		printf("ERROR: Unable to open file\n");
		return;
	}

	// initialize vector
	faces = new std::vector<MFace>();
	vertices = new std::vector<Vector3d>();

	do {
		fgets(buf, 256, file);

		if (buf[0] == '#') 
			continue; // skip this comment line

		if ( (inner = strstr(buf, "Vertex")) != NULL) {
			// Process this line as a vertex declaration
			inner += 6; 
			pch=strtok(inner, " \t"); // this also gets the first integer token
									  // which is to be skipped

			// scan three consecutive floats
			for (i = 0; i < 3; i++) {
				pch = strtok(NULL, " \t");
				if (pch == NULL)
					break;

				// scan in the float value
				sscanf(pch, "%f", &xyz[i]);
			}

			if (i < 3) {
				printf("Error: Could not find 3 float values.\n");
				goto err;
			}

			// add these floats as a vertex
			vertices->push_back(Vector3d(xyz[0], xyz[1], xyz[2], vertices->size()));
		}
		else if ( (inner=strstr(buf, "Face")) != NULL) {
			// Process this line as a face declaration
			inner += 4;
			pch = strtok(inner, " \t"); // this also gets the first integer token
										// which is to be skipped

			// scan three consecutive ints
			for (i = 0; i < 3; i++) {
				pch = strtok(NULL, " \t");
				if (pch == NULL)
					break;

				// scan in the float value
				sscanf(pch, "%d", &tempFace.indices[i]);
				tempFace.indices[i]--; // Zero-based indexing adjustment
			}

			if (i < 3) {
				printf("Error: Could not find 3 int values.\n[%s]\n", buf);
				goto err;
			}

			// add this face into the list
			faces->push_back(tempFace);
		}
		else {
			printf("Unknown operation in MFile. Line ignored.\n[%s]\n", buf);
			continue;
		}

		buf[0] = 0;
	} while (!feof(file));

	fclose(file);
	loaded = 1;
	printf("File successfully loaded!\nVertex Count\t: %d\nFace Count\t: %d\n", getVertexCount(), getFaceCount());
	return;

err:
	if(file!=NULL)
		fclose(file);
}

MFile::~MFile() {
	if (faces != NULL)
	{
		delete faces;
		faces = NULL;
	}

	if (vertices != NULL)
	{
		delete vertices;
		vertices = NULL;
	}

}

int MFile::isLoaded() {
	return loaded;
}

int MFile::getFaceCount() {
	return faces->size();
}

int MFile::getVertexCount() {
	return vertices->size();
}

MFace* MFile::getFace(int index) {
	return &(faces->at(index));
}

Vector3d* MFile::getVertex(int index) {
	return &(vertices->at(index));
}

std::vector<Vector3d>** MFile::getVertexList() {
	return &vertices;
}
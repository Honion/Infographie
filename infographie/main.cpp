#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "geometry.h"


using namespace std;


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

//Algorithme de Brensenham
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0-x1)<std::abs(y0-y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0>x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

    int dx = x1-x0;
    int dy = y1-y0;
    int derror = std::abs(dy)*2;
    int error = 0;
    int y = y0;

    for (int x=x0; x<=x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error += derror;
        if (error > dx) {
            y += (y1>y0?1:-1);
            error -= dx*2;
        }
    }
}

void load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}



TGAImage dessin(TGAImage image,float **buffer, TGAImage diffusemap){
   for(int i = 0; i<1000 ; i++){
      for(int j = 0 ; j<1000 ; j++){
        if( buffer[i][j]!=NULL)
            {
                image.set(i, j, TGAColor(diffusemap.get(i,j).r*buffer[i][j], diffusemap.get(i,j).g*buffer[i][j], diffusemap.get(i,j).b*buffer[i][j],255));
            }
      }
   }
    return image;
}


float** supprZone(int x1, int y1, int x2, int y2, int x3, int y3,float intensity, float **buffer ){
   for(int i = 0; i<1000 ; i++){
      for(int j = 0 ; j<1000 ; j++){
        if((((x1-i)*(y2-j)-(y1-j)*(x2-i))>0 && ((x2-i)*(y3-j)-(y2-j)*(x3-i))>0 && ((x3-i)*(y1-j)-(y3-j)*(x1-i))>0 ) || (((x1-i)*(y2-j)-(y1-j)*(x2-i))<0 && ((x2-i)*(y3-j)-(y2-j)*(x3-i))<0 && ((x3-i)*(y1-j)-(y3-j)*(x1-i))<0 )  ){
            if( buffer[i][j]==NULL || intensity > buffer[i][j])
            {
                buffer[i][j]=intensity;
            }
        }
      }
   }

    return buffer ;
}


// recupere les abscisses
double getabscisse(string v) {
	char * pch;
	double res;
	char s[v.size() + 1];
	strcpy(s, v.c_str());

	pch = strtok(s, " ");
	int j = 1;
	while (pch != NULL) {

		if (j == 1) {
			res = strtod(pch, NULL);
		}

		j++;
		pch = strtok(NULL, " ");
	}
	return res;
}

// recupere les ordonnees
double getordonnee(string v) {
	char * pch;
	double res;
	char s[v.size() + 1];
	strcpy(s, v.c_str());

	pch = strtok(s, " ");
	int j = 1;

	while (pch != NULL) {

		if (j == 2) {
			res = strtod(pch, NULL);
		}

		j++;
		pch = strtok(NULL, " ");
	}
	return res;
}

// recupere la profondeur
double getprofondeur(string v) {
    char * pch;
    double res;
    char s[v.size() + 1];
    strcpy(s, v.c_str());

    pch = strtok(s, " ");
    int j = 1;

    while (pch != NULL) {

        if (j == 3) {
            res = strtod(pch, NULL);
        }

        j++;
        pch = strtok(NULL, " ");
    }
    return res;
}

// Dessine les triangles
void creation_triangle(vector<string> tabv, vector<string> tabf, TGAImage diffusemap) {
	TGAImage image(1000, 1000, TGAImage::RGB);
	int point1 = -1, point2 = -1, point3 = -1;
    float **buffer;
    buffer = new float* [ 1000 ];
    for (int i=0; i < 1000; i++)
    buffer[i] = new float[ 1000 ];

	for (int i = 0; i < tabf.size(); i++) {
		string ligne = tabf[i];
		char * pch;
		char s[ligne.size() + 1];
		strcpy(s, ligne.c_str());
		ligne.erase();
		pch = strtok(s, " ");
		int j = 1;
		while (pch != NULL) {
			if (j == 1) {
				point1 = atoi(pch);
			}
			if (j == 2) {
				point2 = atoi(pch);
			}
			if (j == 3) {
				point3 = atoi(pch);
			}

			j++;
			pch = strtok(NULL, " ");
		}

		string v1, v2, v3;
        float x0, x1, x2, y0, y1, y2, z0, z1, z2;



		v1 = tabv[point1 - 1];
		v2 = tabv[point2 - 1];
		v3 = tabv[point3 - 1];

		x0 = (getabscisse(v1) + 1) * 500;
		x1 = (getabscisse(v2) + 1) * 500;
		x2 = (getabscisse(v3) + 1) * 500;


		y0 = (getordonnee(v1) + 1) * 500;
		y1 = (getordonnee(v2) + 1) * 500;
		y2 = (getordonnee(v3) + 1) * 500;


        z0 = (getprofondeur(v1) + 1) * 500;
        z1 = (getprofondeur(v2) + 1) * 500;
        z2 = (getprofondeur(v3) + 1) * 500;

        Vec3f world_coords[3];
        world_coords[0] = { x0,y0,z0 };
        world_coords[1] = { x1,y1,z1 };
        world_coords[2] = { x2,y2,z2 };


        Vec3f light_dir(0,0,-1);
        Vec3f n = (world_coords[2] - world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;



        if(intensity > 0){
             line(x0,y0,x1,y1,image,TGAColor(intensity*255,intensity*255,intensity*255,255));
             line(x1,y1,x2,y2,image,TGAColor(intensity*255,intensity*255,intensity*255,255));
             line(x0,y0,x2,y2,image,TGAColor(intensity*255,intensity*255,intensity*255,255));
             buffer =supprZone(x0,y0,x1,y1,x2,y2,intensity, buffer);
        }
	}
    image = dessin(image, buffer, diffusemap);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("visage.tga");
}

// Lire le fichier objet ( f et v )
int lecture_objet(const std::string & path) {
	fstream fichier(path.c_str());

	TGAImage diffusemap;
    load_texture(path, "_diffuse.tga", diffusemap);

	vector < string > tabf;
	vector < string > tabv;

	if (!fichier) {
		cout << "erreur" << endl;
		return 1;
	}

	else {

		while (!fichier.eof()) {
			string ligne;
			getline(fichier, ligne);

			if (ligne.substr(0, 2) == "f ") {
				ligne = ligne.substr(2, ligne.size());
				string l;

				int i = 0;
				char * pch;
				char s[ligne.size() + 1];
				strcpy(s, ligne.c_str());
				ligne.erase();

				pch = strtok(s, "/");
				while (pch != NULL) {

					if (i == 0) {
						l = l + " " + pch;
					}

					pch = strtok(NULL, "/");
					if (i == 3) {
						i = 0;
					} else {
						i += 1;
					}
				}

				tabf.push_back(l);
			}

			if (ligne.substr(0, 2) == "v ") {
				ligne = ligne.substr(2, ligne.size());
				tabv.push_back(ligne);
			}

		}

	}
	fichier.close();
	creation_triangle(tabv, tabf, diffusemap);
	return 0;
}




int main(int argc, char** argv) {
    TGAImage image(1000, 1000, TGAImage::RGB);

    lecture_objet("./african_head.obj");
    return 0;
}


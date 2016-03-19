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

float* barycentre(float x1, float y1, float x2, float y2, float x3, float y3, int xc, int yc){
   float* x = new float[3];
   float* y = new float[3];
   x[0] = x3-x1;
   x[1] = x2-x1;
   x[2] = x1-xc;
   y[0] = y3-y1;
   y[1] = y2-y1;
   y[2] = y1-yc;

   float* u = new float[3];
   u[0] = (x[1]*y[2])-(x[2]*y[1]);
   u[1] = (x[2]*y[0])-(x[0]*y[2]);
   u[2] = (x[0]*y[1])-(x[1]*y[0]);

   float* u2 = new float[3];
   u2[0] = 1.-(u[0]+u[1])/u[2];
   u2[1] = u[1]/u[2];
   u2[2] = u[0]/u[2];

   return u2;
}



TGAImage dessin(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float xt1, float yt1, float xt2, float yt2, float xt3, float yt3, float intensity, float * zbuffer, TGAImage image, TGAImage diffusemap){
    int minx = std::min(std::min(x1 , x2), x3);
    int miny = std::min(std::min(y1 , y2 ), y3);
    int maxx = std::max(std::max(x1 , x2), x3);
    int maxy = std::max(std::max(y1 , y2 ), y3);

    minx = std::max(0, minx);
    miny = std::max(0, miny);
    maxx = std::min(image.get_width()-1, maxx);
    maxy = std::min(image.get_height()-1, maxy);

    for (int i=minx; i<=maxx; i++){
        for (int j=miny; j<=maxy; j++){
            float* bar = new float[3];
            bar = barycentre(x1, y1, x2, y2, x3, y3, i, j);
            if ( (((x1-i)*(y2-j)-(y1-j)*(x2-i))>=0 && ((x2-i)*(y3-j)-(y2-j)*(x3-i))>=0 && ((x3-i)*(y1-j)-(y3-j)*(x1-i))>=0 ) || (((x1-i)*(y2-j)-(y1-j)*(x2-i))<=0 && ((x2-i)*(y3-j)-(y2-j)*(x3-i))<=0 && ((x3-i)*(y1-j)-(y3-j)*(x1-i))<=0 ) ) {
                float z = bar[0]*z1 + bar[1]*z2 + bar[2]*z3;
                if ( z > zbuffer[i+j*1000] ) {
                    zbuffer[i+j*1000] = z;
                    int x = (bar[0]*xt1 + bar[1]*xt2 + bar[2]*xt3) * 1024;
                    int y = (bar[0]*yt1 + bar[1]*yt2 + bar[2]*yt3) * 1024;
                    image.set(i, j, TGAColor(diffusemap.get(x,y).r*intensity, diffusemap.get(x,y).g*intensity, diffusemap.get(x,y).b*intensity,255));
                }
            }
        }
    }
    return image;
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
void creation_triangle(vector<string> tabv, vector<string> tabf, vector<string> tabvt ,  vector<string> tabft,  TGAImage diffusemap) {
	TGAImage image(1000, 1000, TGAImage::RGB);
	int point1 = -1, point2 = -1, point3 = -1, pointv1 = -1, pointv2 = -1, pointv3 = -1;
	float* zbuffer = new float[1000*1000];
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

		string ligne2 = tabft[i];
		char * pch2;
		char s2[ligne2.size() + 1];
		strcpy(s2, ligne2.c_str());
		ligne2.erase();
		pch2 = strtok(s2, " ");
		int k = 1;
		while (pch2 != NULL) {
			if (k == 1) {
				pointv1 = atoi(pch2);
			}
			if (k == 2) {
				pointv2 = atoi(pch2);
			}
			if (k == 3) {
				pointv3 = atoi(pch2);
			}

			k++;
			pch2 = strtok(NULL, " ");
		}


		string v1, v2, v3, vt1, vt2, vt3;
        float x0, x1, x2, y0, y1, y2, z0, z1, z2, xt0, xt1, xt2, yt0, yt1, yt2;



		v1 = tabv[point1 - 1];
		v2 = tabv[point2 - 1];
		v3 = tabv[point3 - 1];

		vt1 = tabvt[pointv1 - 1];
		vt2 = tabvt[pointv2 - 1];
		vt3 = tabvt[pointv3 - 1];



		x0 = (getabscisse(v1) + 1) * 500;
		x1 = (getabscisse(v2) + 1) * 500;
		x2 = (getabscisse(v3) + 1) * 500;

		xt0 = getabscisse(vt1);
		xt1 = getabscisse(vt2);
		xt2 = getabscisse(vt3);



		y0 = (getordonnee(v1) + 1) * 500;
		y1 = (getordonnee(v2) + 1) * 500;
		y2 = (getordonnee(v3) + 1) * 500;

		yt0 = getordonnee(vt1);
		yt1 = getordonnee(vt2);
		yt2 = getordonnee(vt3);


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
             image = dessin(x0,y0,z0,x1,y1,z1,x2,y2,z2,xt0,yt0,xt1,yt1,xt2,yt2,intensity, zbuffer, image, diffusemap);
        }
	}
	image.flip_vertically();
	image.write_tga_file("visage.tga");
}

// Lire le fichier objet ( f et v )
int lecture_objet(const std::string & path) {
	fstream fichier(path.c_str());

	TGAImage diffusemap;
    load_texture(path, "_diffuse.tga", diffusemap);

	vector < string > tabf;
	vector < string > tabft;
	vector < string > tabv;
	vector < string > tabvt;

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

				string l2;
				int i2 = 0;


				char * pch;
				char s[ligne.size() + 1];
				strcpy(s, ligne.c_str());

				char * pch2;
				char s2[ligne.size() + 1];
				strcpy(s2, ligne.c_str());

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
				pch2 = strtok(s2, "/");
				while (pch2 != NULL) {

					if (i2 == 1) {
						l2 = l2 + " " + pch2;
					}

					pch2 = strtok(NULL, "/");
					if (i2 == 2) {
						i2 = 1;
					} else {
						i2 += 1;
					}
				}
				tabf.push_back(l);
				tabft.push_back(l2);
			}

			if (ligne.substr(0, 2) == "v ") {
				ligne = ligne.substr(2, ligne.size());
				tabv.push_back(ligne);
			}

			if (ligne.substr(0, 2) == "vt") {
				ligne = ligne.substr(2, ligne.size());
				tabvt.push_back(ligne);
			}

		}


	}
	fichier.close();
	creation_triangle(tabv, tabf, tabvt, tabft, diffusemap);
	return 0;
}




int main(int argc, char** argv) {
    TGAImage image(1000, 1000, TGAImage::RGB);

    lecture_objet("./african_head.obj");
    return 0;
}


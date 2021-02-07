#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

#ifdef MAC_OS
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif // MAC_OS

#include "Viewer/Arcball.h" /* Arc Ball Interface */

#include "MyMesh.h"
#include "MyTMesh.h"
#include "HandleTunnelLoop.h"

using namespace DartLib;

std::string mesh_name;
std::vector<int> user_dis;
std::string inp;
std::istringstream iss;
std::string line;
std::ifstream myfile;

/* window width and height */
int win_width, win_height;
int gButton;
int startx, starty;
int shadeFlag = 0;
int displayMode = 0;
int displayModel = 1;   // mesh or tmesh
bool switchDirection = true;

/* rotation quaternion and translation vector for the object */
CQrot ObjRot(0, 0, 1, 0);
CPoint ObjTrans(0, 0, 0);


CMyTMesh mesh;
CMyTMesh mesh2;
CHandleTunnelLoop handler;

std::set<CMyTMesh::CEdge*>* boundary_edges;
std::vector<CMyTMesh::CFace*> boundary_surface;
CMyMesh boundary_mesh;
std::string g_output;


bool exterior_volume = false;

/* arcball object */
CArcball arcball;

// which loop to display
int which = 0;
int pIndex = 0;
int pIndex2 = 0;
int pIndex3 = 0;
int which_edge = 0;

/*! setup the object, transform from the world to the object coordinate system */
void setupObject(void)
{
    double rot[16];

    glTranslated(ObjTrans[0], ObjTrans[1], ObjTrans[2]);
    ObjRot.convert(rot);
    glMultMatrixd((GLdouble*) rot);
}

/*! the eye is always fixed at world z = +5 */

void setupEye(void)
{
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

/*! setup light */
void setupLight()
{
    CPoint position(0, 0, 1);
    GLfloat lightOnePosition[4] = {position[0], position[1], position[2], 0};
    glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
}

void draw_face(CMyTMesh::CFace* face, bool inv = false)
{
    CPoint n = face->normal();

    std::vector<CMyTMesh::CVertex*> verts;
    for (CMyTMesh::FaceVertexIterator fviter(face); !fviter.end(); ++fviter)
    {
        CMyTMesh::CVertex* pV = *fviter;
        verts.push_back(pV);
    }

    if (inv)
    {
        std::reverse(verts.begin(), verts.end());
        n = n * (-1);
    }

    glBegin(GL_TRIANGLES);
    for (auto& pV : verts)
    {
        CPoint& p = pV->point();
        if (!switchDirection)
            glNormal3d(n[0], n[1], n[2]);
        else
            glNormal3d(-n[0], -n[1], -n[2]);
        glVertex3d(p[0], p[1], p[2]);
    }
    glEnd();
}

void draw_halffaces(std::vector<CMyTMesh::CDart*>& darts, GLenum mode) 
{
    glColor3f(1.0, 0.5, 0.0);
    glPolygonMode(GL_FRONT, mode);

    for (CMyTMesh::CDart* pD : darts)
    {
        auto f = mesh.C2(pD);
        CMyTMesh::CDart* darts_on_f[3];
        darts_on_f[0] = mesh.D(f);
        darts_on_f[1] = mesh.beta(1, darts_on_f[0]);
        darts_on_f[2] = mesh.beta(1, darts_on_f[1]);
        
        if (darts_on_f[0] == pD ||
            darts_on_f[1] == pD ||
            darts_on_f[2] == pD)
        {
            // TODO: I think `draw_face(f, false)` should work well here,
            //       but I must do as follows. 
            if (!mesh.boundary(pD)) // inner face
                draw_face(f, true);
            else
                draw_face(f, false);
        }
        else
        {
            // TODO: I think `draw_face(f, true)` should work well here,
            //       but I must do as follows. 
            if (!mesh.boundary(pD)) // inner face
                draw_face(f, false);
            else
                draw_face(f, true);
        }
    }
}

void draw_sharp_edges(CMyTMesh* pMesh)
{
    glDisable(GL_LIGHTING);
    glLineWidth(3.0);
    glBegin(GL_LINES);

    for (CMyTMesh::EdgeIterator eiter(pMesh); !eiter.end(); eiter++)
    {
        CMyTMesh::CEdge* pE = *eiter;
        if (!pE->sharp() && !pE->green())
            continue;
		if (pE->green())
			glColor3f(1.0, 1.0, 0.0);
        else if (boundary_edges->find(pE) != boundary_edges->end())
            glColor3f(1.0, 0.0, 0.0);
        else
            glColor3f(0.0, 0.0, 1.0);

        CMyTMesh::CVertex* pv1 = pMesh->edge_vertex(pE, 0);
        CMyTMesh::CVertex* pv2 = pMesh->edge_vertex(pE, 1);

        CPoint p1 = pv1->point();
        CPoint p2 = pv2->point();

        glVertex3d(p1[0], p1[1], p1[2]);
        glVertex3d(p2[0], p2[1], p2[2]);
    }
    glEnd();
    glLineWidth(0.5);
    glEnable(GL_LIGHTING);
}

void draw_boundary_surface() 
{
    glEnable(GL_LIGHTING);
    glLineWidth(0.5);
    glColor3f(229.0 / 255.0, 162.0 / 255.0, 141.0 / 255.0);

    for (auto pF : boundary_surface)
    {
        glBegin(GL_POLYGON);
        for (CMyTMesh::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            CMyTMesh::CVertex* pV = *fviter;
            CPoint& p = pV->point();
            CPoint & n = pF->normal();

            if (!switchDirection)
                glNormal3d(n[0], n[1], n[2]);
            else
                glNormal3d(-n[0], -n[1], -n[2]);

            glVertex3d(p[0], p[1], p[2]);
        }
        glEnd();
    }
}

void draw_boundary_sharp_edges()
{
    glDisable(GL_LIGHTING);
    glLineWidth(3.0);
    glBegin(GL_LINES);

    for (auto pF : boundary_surface)
    {
        for (CMyTMesh::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
        {
            CMyTMesh::CEdge* pE = *feiter;
            

			if (pE->green())
			{
				glColor3f(0.0, 1.0, 0.0);

				CMyTMesh::CVertex* pv1 = mesh.edge_vertex(pE, 0);
				CMyTMesh::CVertex* pv2 = mesh.edge_vertex(pE, 1);

				CPoint p1 = pv1->point();
				CPoint p2 = pv2->point();

				glVertex3d(p1[0], p1[1], p1[2]);
				glVertex3d(p2[0], p2[1], p2[2]);
				continue;
			}
			if (!pE->sharp()) continue;
            glColor3f(0.0, 1.0, 0.0);

            CMyTMesh::CVertex* pv1 = mesh.edge_vertex(pE, 0);
            CMyTMesh::CVertex* pv2 = mesh.edge_vertex(pE, 1);

            CPoint p1 = pv1->point();
            CPoint p2 = pv2->point();

            glVertex3d(p1[0], p1[1], p1[2]);
            glVertex3d(p2[0], p2[1], p2[2]);
        }
    }

    glEnd();
    glLineWidth(0.5);
    glEnable(GL_LIGHTING);
}

/*! draw mesh */
void draw_mesh()
{
    if (displayModel == 1)
    {

        switch (displayMode)
        {
            case 0:
                draw_halffaces(mesh.halffaces_below(), GL_FILL);
                break;
            case 1:
                draw_halffaces(mesh.halffaces_below(), GL_LINE);
                break;
            case 2:
                draw_halffaces(mesh.halffaces_below(), GL_FILL);
                draw_halffaces(mesh.halffaces_above(), GL_LINE);
                break;
        }
        draw_sharp_edges(&mesh);
    }
    else
    {
        draw_boundary_surface();
        draw_boundary_sharp_edges();
    }
}

/*! display call back function
 */
void display()
{
    /* clear frame buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupLight();
    /* transform from the eye coordinate system to the world system */
    setupEye();
    glPushMatrix();
    /* transform from the world to the ojbect coordinate system */
    setupObject();
    /* draw the mesh */
    draw_mesh();
    glPopMatrix();
    glutSwapBuffers();
}

/*! Called when a "resize" event is received by the window. */

void reshape(int w, int h)
{
    float ar;

    win_width = w;
    win_height = h;

    ar = (float) (w) / h;
    glViewport(0, 0, w, h); /* Set Viewport */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // magic imageing commands
    gluPerspective(40.0, /* field of view in degrees */
                   ar,   /* aspect ratio */
                   0.01,  /* Z near */
                   100.0 /* Z far */);

    glMatrixMode(GL_MODELVIEW);

    glutPostRedisplay();
}

/*! helper function to remind the user about commands, hot keys */
void help()
{
    printf("w  -  wireframe Display\n");
    printf("f  -  Flat Shading\n");
    printf("g  -  display mode for volume mesh\n");
    printf("b  -  display volume mesh or its boundary surface\n");
    printf("+  -  shfit the cutting plane upwards\n");
    printf("-  -  shfit the cutting plane downwards\n");
    printf("x  -  set the cutting plane normal (1,0,0)\n");
    printf("y  -  set the cutting plane normal (0,1,0)\n");
    printf("z  -  set the cutting plane normal (0,0,1)\n");
    printf("G  -  compute the handle, tunnel loops\n");
    printf("?  -  help Information\n");
    printf("esc - quit\n");
}

/*! Keyboard call back function */

void keyBoard(unsigned char key, int x, int y)
{
    static double d = 0;
    static CPoint n(0, 0, 1);

    CMyTMesh::CBoundary boundary(&mesh);
    clock_t begin, end;
	

    switch (key)
    {
        case '+':
            d = std::min(d + 0.05, 2.0);
            mesh.cut(CPlane(n, d));
            break;
        case '-':
			d = std::max(d - 0.05, -2.0);
            mesh.cut(CPlane(n, d));
            break;
        case 'x':
            n = CPoint(1, 0, 0);
            mesh.cut(CPlane(n, d));
            break;
        case 'y':
            n = CPoint(0, 1, 0);
            mesh.cut(CPlane(n, d));
            break;
        case 'z':
            n = CPoint(0, 0, 1);
            mesh.cut(CPlane(n, d));
            break;
        case 'f':
            // Flat Shading
            glPolygonMode(GL_FRONT, GL_FILL);
            shadeFlag = 0;
            break;
        case 's':
            // Smooth Shading
            glPolygonMode(GL_FRONT, GL_FILL);
            shadeFlag = 1;
            break;
        case 'w':
            // Wireframe mode
            glPolygonMode(GL_FRONT, GL_LINE);
            break;
        case 'g':
            displayMode = (displayMode + 1) % 3;
            break;
        case 'b':
            displayModel = (displayModel + 1) % 2;
            break;
        case 'r':
            switchDirection = !switchDirection;
            if (switchDirection)
                glFrontFace(GL_CW);
            else 
                glFrontFace(GL_CCW);
            break;
        case 'G':
            begin = clock();
			if (exterior_volume)
			{
				handler.setExterior();
			}
			if (exterior_volume)
			{
				handler.boundary_surface_pair();
				handler.interior_volume_pair();
				if (exterior_volume)
				{
					handler.write_tunnels("../../data/tunnels.txt");
				}
				boundary_edges = &handler.boundary_edges();

				handler.exact_boundary(boundary_mesh);

				if (!g_output.empty())
					handler.write_m(g_output);

				end = clock();
				printf("Computing handle/tunnel loops time: %g s\n", double(end - begin) / CLOCKS_PER_SEC);
			}
			else
			{
				boundary_edges = &handler.boundary_edges();

				handler.exact_boundary(boundary_mesh);
				myfile.open("../../data/tunnels.txt");
				if (myfile.is_open())
				{
					while (std::getline(myfile, line))
					{
						handler.add_tunnel(line);
					}
					handler.start_shorten2();
					myfile.close();
					
				}
				else
				{
					std::cout << "Unable to open tunnels file";
				}
				end = clock();
				printf("Shortening tunnel loops time: %g s\n", double(end - begin) / CLOCKS_PER_SEC);
			}
			break;
		case 'P':
			handler.prune();
			break;
		case 'S':
			handler.next_shorten_step();
			break;
		case 'X':
			handler.display_individual(which_edge);
			which_edge += 1;
			break;
		case 'L':
			handler.go_back();
			break;
		case ':':
			handler.go_forward();
			break;
		case 'D':
			handler.display_all_after();
			break;
		case 'd':
			handler.display_all_before();
			break;
		case 'm':
			handler.display_all_middle();
			break;
		case 'B':
			handler.display_before(which);
			which += 1;
			break;
		case 'A':
			handler.display_tested(which);
			which += 1;
			break;
		case 'p':
			handler.display_all_before_prune();
			break;
		case 'N':
			which += 1;
			break;
		case 'W':
			handler.display_all_before();
			handler.write_m("../../data/left_before_loops.txt");
			handler.display_all_after();
			handler.write_m("../../data/left_after_loops.txt");
			break;
		case 'E':
			handler.write_after_obj("../../data/left_after_loops.obj");
			break;
		//case 'R':
			//handler.write_before_ply("../../data/left_before_loops.ply");
			//break;
		case 'R':
			handler.write_before_obj("../../data/left_before_loops.obj");
			break;
		case 'T':
			handler.write_good_after_obj("../../data/left_good_after_loops.obj");
			break;
		case 'O':
			handler.show_original();
			break;
		case 'Q':
			handler.show_starting(which);
			break;
		case 'M':
			user_dis.clear();
			std::cin >> inp;
			while (inp != "s")
			{
				int d = std::stoi(inp);
				user_dis.push_back(d);
				std::cin >> inp;
			}
			handler.display_loop(user_dis);
			break;
		case '<':
			user_dis.clear();
			std::getline(std::cin, inp);
			iss.str(inp);
			int n;
			while (iss >> n) 
			{
				user_dis.push_back(n);
			}
			handler.display_edges(user_dis);
			break;
		case '.':
			handler.display_pair(pIndex);
			pIndex += 1;
			break;
		case '/':
			handler.display_generated_loop(pIndex2);
			pIndex2 += 1;
			break;
		case '>':
			handler.display_correct_loop(pIndex3);
			pIndex3 += 1;
			break;

        case '?':
            help();
            break;
        case 27:
            exit(0);
            break;
    }

    glutPostRedisplay();
}

/*! setup GL states */
void setupGLstate()
{

    GLfloat lightOneColor[] = {1, 1, 1, 1};
    GLfloat globalAmb[] = {.1, .1, .1, 1};
    GLfloat lightOnePosition[] = {.0, .0, 1, 0.0};

    glEnable(GL_CULL_FACE);
    if (switchDirection) glFrontFace(GL_CW);
    else                 glFrontFace(GL_CCW);
            
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3, 0.3, 0.8, 0.3);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);

    GLfloat diffuseMaterial[4] = {0.5, 0.5, 0.5, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};

    glEnable(GL_DEPTH_TEST);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 25.0);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

/*! mouse click call back function */
void mouseClick(int button, int state, int x, int y)
{

    /* set up an arcball around the Eye's center
            switch y coordinates to right handed system  */

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        gButton = GLUT_LEFT_BUTTON;
        arcball = CArcball(win_width, win_height, x - win_width / 2, win_height - y - win_height / 2);
    }

    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
        startx = x;
        starty = y;
        gButton = GLUT_MIDDLE_BUTTON;
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        startx = x;
        starty = y;
        gButton = GLUT_RIGHT_BUTTON;
    }
    return;
}

/*! mouse motion call back function */
void mouseMove(int x, int y)
{
    CPoint trans;
    CQrot rot;

    /* rotation, call arcball */
    if (gButton == GLUT_LEFT_BUTTON)
    {
        rot = arcball.update(x - win_width / 2, win_height - y - win_height / 2);
        ObjRot = rot * ObjRot;
        glutPostRedisplay();
    }

    /*xy translation */
    if (gButton == GLUT_MIDDLE_BUTTON)
    {
        double scale = 10. / win_height;
        trans = CPoint(scale * (x - startx), scale * (starty - y), 0);
        startx = x;
        starty = y;
        ObjTrans = ObjTrans + trans;
        glutPostRedisplay();
    }

    /* zoom in and out */
    if (gButton == GLUT_RIGHT_BUTTON)
    {
        double scale = 10. / win_height;
        trans = CPoint(0, 0, scale * (starty - y));
        startx = x;
        starty = y;
        ObjTrans = ObjTrans + trans;
        glutPostRedisplay();
    }
}

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        printf("Usage: %s mesh_name\n", argv[0]);
        return EXIT_FAILURE;
    }

    mesh_name = argv[1];
    
    if (strutil::endsWith(mesh_name, ".t"))
    {
		if (strutil::endsWith(mesh_name, "O.t"))
		{
			exterior_volume = true;
		}
        
    }
    else
    {
        printf("Only support .t file now!\n");
        exit(EXIT_FAILURE);
    }
	
	handler.set_name(mesh_name);
	if (strutil::endsWith(mesh_name, "_2_I.t"))
	{
		clock_t begin = clock();


		int reload_mesh = 0;

		myfile.open("../../data/tunnels.txt");
		
		if (myfile.is_open())
		{
			while (std::getline(myfile, line))
			{
				if (reload_mesh % 5 == 0)
				{
					mesh.load_t(argv[1]);
					CMyTMesh::CBoundary boundary(&mesh);
					boundary_surface = boundary.boundary_surface();
					mesh.normalize();
					mesh.compute_face_normal();
					CPlane p(CPoint(0, 0, 1), 0);
					mesh.cut(p);
					handler.set_mesh(&mesh);
					boundary_edges = &handler.boundary_edges();
					handler.exact_boundary(boundary_mesh);
				}
				reload_mesh += 1;
				std::cout << "started one shortening\n";
				handler.add_tunnel(line);
				handler.start_shorten();
				std::cout << "finished one shortening\n";
			}
			myfile.close();
		}
		mesh.load_t(argv[1]);
		CMyTMesh::CBoundary boundary(&mesh);
		boundary_surface = boundary.boundary_surface();
		mesh.normalize();
		mesh.compute_face_normal();
		CPlane p(CPoint(0, 0, 1), 0);
		mesh.cut(p);
		handler.set_mesh(&mesh);
		boundary_edges = &handler.boundary_edges();
		handler.exact_boundary(boundary_mesh);
		clock_t end = clock();
		printf("Putting in all the tets time: %g s\n", double(end - begin) / CLOCKS_PER_SEC);
	}
	else
	{
		clock_t begin = clock();

		mesh.load_t(argv[1]);

		clock_t end = clock();
		printf("Load time: %g s\n", double(end - begin) / CLOCKS_PER_SEC);
		g_output = argc > 2 ? std::string(argv[2]) : "";

		CMyTMesh::CBoundary boundary(&mesh);
		boundary_surface = boundary.boundary_surface();


		mesh.normalize();
		mesh.compute_face_normal();

		CPlane p(CPoint(0, 0, 1), 0);
		mesh.cut(p);

		handler.set_mesh(&mesh);
	}
    /* glut stuff */
    glutInit(&argc, argv); /* Initialize GLUT */
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Mesh Viewer"); /* Create window with given title */
    glViewport(0, 0, 10000, 10000);

    glutDisplayFunc(display); /* Set-up callback functions */
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutKeyboardFunc(keyBoard);
    setupGLstate();

    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}

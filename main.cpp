/*
 * Copyright BMPI 2011
 * J.W. Staley - MIRA, 
 *               Biomedical Photonics Imaging Group (BMPI), 
 *				 University of Twente
 *
 */




#include "stdio.h"
#include "photon.h"
#include "medium.h"
#include "layer.h"
#include "sphereAbsorber.h"
#include "cylinderAbsorber.h"
#include "coordinates.h"
#include "vector3D.h"
#include "vectorMath.h"
#include "logger.h"
#include "circularDetector.h"
#include <cmath>
#include <time.h>
#include <vector>
#include <boost/thread/thread.hpp> 
#include <string>
#include <iostream>
using std::cout;
using std::endl;



const int MAX_PHOTONS = 10000;

//#define DEBUG 1



int main()
{
    
    // The logger is a singleton.  To bypass any problems with using singletons in a multi-threaded applicaton
    // initialization occurs in main before any threads are spawned.
    std::string file = "exit-locations.txt";
    Logger::getInstance()->openExitFile(file);
    file = "absorber-data.txt";
    Logger::getInstance()->openAbsorberFile(file);

    /*
    boost::shared_ptr<Vector3d> p0(new Vector3d(2.0f, 1.0f, 1.0f));
    boost::shared_ptr<Vector3d> p1(new Vector3d(3.5f, 1.5f, 11.0f));
    boost::shared_ptr<Vector3d> dir;
    boost::shared_ptr<Vector3d> c0(new Vector3d(0.0f, 0.0f, 11.0f));
    boost::shared_ptr<Vector3d> c1(new Vector3d(2.0f, 3.0f, 11.0f));
    boost::shared_ptr<Vector3d> c2(new Vector3d(11.0f, 13.5f, 11.0f));
    boost::shared_ptr<Vector3d> n;
    
    
    n = VectorMath::crossProduct((*c1 - *c0), (*c2 - *c0));
    //n.reset(new Vector3d(1.0f, 2.0f, 3.0f));
    VectorMath::Normalize(n);
    
    double u = VectorMath::dotProduct(n, (*c0 - *p0)) / VectorMath::dotProduct(n, (*p1 - *p0));
    double THRESH = 0.0000000000001;
    if (u < 0.0f || u > 1.0f + THRESH)
        cout << "FALSE\n";
    
    cout << "n = " << n;
    cout << "u = " << u << endl;
    
    
    double z0 = p0->location.z;
    double z1 = p1->location.z;
    
    double y0 = p0->location.y;
    double y1 = p1->location.y;
    
    double x0 = p0->location.x;
    double x1 = p1->location.x;
    
    double distToPlane = abs(VectorMath::dotProduct(n, (*c0-*p0)) / VectorMath::Length(n));
    //D = VectorMath::Distance(c0, p0);
    cout << "distance to plane = " << distToPlane << endl;
    cout << (*c0 - *p0);
    
    double z= z0 + (z1-z0)*u;
    double y = y0 + (y1-y0)*u;
    double x = x0 + (x1-x0)*u;
    
    boost::shared_ptr<Vector3d> intersectPoint(new Vector3d(x, y, z));
    cout << "intersection point = " << intersectPoint;
    
    
    CircularDetector detector(1.0f, Vector3d(1.0f, 1.0f, 11.0f));
    detector.setDetectorPlaneXY();  // Set the plane the detector is orientated on.
    bool hitDetector = detector.photonPassedThroughDetector(p0, p1);
    cout << "hitDetector = " << hitDetector << endl;
    */
    
//    z = (*p) - (*x);
//    cout << "p - x = " << z->X() << endl;
//    
//    using namespace VectorMath;
//    z = VectorMath::crossProduct(p, x);
//    double d = VectorMath::dotProduct(p, x);
//    cout << "Dot product = " << d << endl;
//    VectorMath::Length(z);
//    
//    //VectorMath vmath;
//    //z = vmath.crossProduct(p, x);
//    cout << "cross x = " << z->location.x << "\ncross y = " << z->location.y << "\ncross z = " << z->location.z << endl;
//    

    

	// The dimensions of the medium.
    double X_dim = 2.0; // [cm]
    double Y_dim = 2.0; // [cm]
    double Z_dim = 2.0; // [cm]
    
	// Create the medium in which the photons will be fired.
	Medium *tissue = new Medium(X_dim, Y_dim, Z_dim);
	

    // Define an air layer.
    double mu_a = 0.0;
    double mu_s = 0.001;
    double refractive_index = 1.0;
    double anisotropy = 1.0;
    double start_depth = 0.0f; // [cm]
    double end_depth = 0.1f; // [cm]
    Layer *airLayer = new Layer(mu_a, mu_s, refractive_index, anisotropy, start_depth, end_depth);
    
    // Define a layer in the tissue.
    mu_a = 0.1;
    mu_s = 7.3;
    refractive_index = 1.33;
    anisotropy = 0.9;
    start_depth = end_depth; // [cm]
    end_depth = Z_dim;   // [cm]
    Layer *tissueLayer1 = new Layer(mu_a, mu_s, refractive_index, anisotropy, start_depth, end_depth);
    
    // Define a spherical absorber.
    SphereAbsorber *absorber0 = new SphereAbsorber(0.6, 1.0, 1.0, 1.0);
    //SphereAbsorber *absorber1 = new SphereAbsorber(0.5, 1.0, 1.5, 1.0);
    absorber0->setAbsorberAbsorptionCoeff(2.0f);
    absorber0->setAbsorberScatterCoeff(mu_s);
    tissueLayer1->addAbsorber(absorber0);
    
    // Create a spherical detector.
    Detector *detector;
    CircularDetector circularExitDetector(1.0f, Vector3d(X_dim/2, Y_dim/2, Z_dim));
    circularExitDetector.setDetectorPlaneXY();  // Set the plane the detector is orientated on.
    detector = &circularExitDetector;
    
    
    // Add the layers to the medium.
    tissue->addLayer(airLayer);
    tissue->addLayer(tissueLayer1);
    tissue->addDetector(detector);
    
    // Define the initial location of injection of the photons.
    coords injectionCoords;
    injectionCoords.x = X_dim/2; // Centered
    injectionCoords.y = Y_dim/2; // Centered
    injectionCoords.z = 0.00001;   // Just below the surface of the 'air' layer.
	
    
	
	// Allocate the planar grid and set it in the tissue.
	double *Cplanar = (double*)malloc(sizeof(double) * 101);
	tissue->setPlanarArray(Cplanar);
		
	// Notify user of execution.
	cout << "Launching photons (" << MAX_PHOTONS << ")...\n";

	// Capture the time before launching photons into the medium.
	clock_t start, end;
	start = clock();
	
	// Let boost decide how many threads to run on this architecture.
	//const int NUM_THREADS = boost::thread::hardware_concurrency();
	const int NUM_THREADS = 1;
    
	// Each thread needs it's own photon object to run, so we need to create
	// an equal amount of photon objects as threads.
	const int NUM_PHOTON_OBJECTS = NUM_THREADS;
    
    // Photon array.
	Photon photons[NUM_PHOTON_OBJECTS];
	boost::thread threads[NUM_THREADS];

	// Init the random number generator.
	srand(time(0));

	// Used to seed the RNG.
	unsigned int s1, s2, s3, s4;

	// Create the threads and give them photon objects to run.
	// Each photon object is run MAX_PHOTONS/NUM_THREADS times, which essentially
	// splits up the work (i.e. photon propagation) amongst many workers.
	for (int i = 0; i < NUM_PHOTON_OBJECTS; i++)
	{
		s1 = rand() + 128;
		s2 = rand() + 128;
		s3 = rand() + 128;
		s4 = rand() + 128;

		cout << "Launching photon object" << i << " iterations: " << MAX_PHOTONS/NUM_THREADS << endl;
		threads[i] = boost::thread(&Photon::injectPhoton, &photons[i], tissue, MAX_PHOTONS/NUM_THREADS,
									s1, s2, s3, s4, injectionCoords);

	}

	// Join all created threads once they have done their work.
	for (int i = 0; i < NUM_PHOTON_OBJECTS; i++)
	{
		threads[i].join();
	}

	
	// Print out the elapsed time it took from beginning to end.
	end = ((double)clock() - start) / CLOCKS_PER_SEC;
	cout << "Time elapsed: " << end << endl;

     
	// Print the matrix of the photon absorptions to file.
	//tissue->printGrid(MAX_PHOTONS);
	
	// Clean up memory allocated memory on the heap.
	delete tissue;
    //delete airLayer;
    //delete tissueLayer1;
    //delete tissueLayer2;

     
	return 0;
}

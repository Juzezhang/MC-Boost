// Class defines the properties of a photon.
#ifndef PHOTON_H
#define PHOTON_H

#include "medium.h"
#include "layer.h"
#include "coordinates.h"
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

#define ALIVE 1		// Value depicting Photon should continue propagation.
#define DEAD  0	    // Photon has lost all energy and failed roulette.

#define ONE_MINUS_COSZERO 1.0E-12
/* If 1-cos(theta) <= ONE_MINUS_COSZERO, fabs(theta) <= 1e-6 rad. */
/* If 1+cos(theta) <= ONE_MINUS_COSZERO, fabs(PI-theta) <= 1e-6 rad. */

#define THRESHOLD	0.01		// Threshold for determining if we should perform roulette
#define CHANCE      0.1  		// Used in roulette
#define PI			3.14159265

#define SIGN(x)           ((x)>=0 ? 1:-1)




//typedef struct coords InjectionCoords;

class Photon
{
public:
	// Constructors
	Photon(void);
	Photon(double x, double y, double z,
		   double dirx, double diry, double dirz);
	// Destructor
	~Photon(void);
    
    // Common function to initialize basic values of the photon object.
    void    initCommon(void);
	
	// Set the number of iterations this Photon (i.e. thread) will run.
	void	setIterations(const int n);

	// Move photon to new position
	void	hop(void);

	// Drop absorbed energy from photon weight due to absorption in medium.
	void	drop(void);

	// Change the trajectory of the photon due to scattering in the medium.
	void	spin(void);
	
	// Set the step size of the photon.
	void 	setStepSize(void);

	// Decide whether the photon should be transmitted to another layer
	// or internally reflected.
	void	transmitOrReflect(const char *);

	// Reset the Photon attributes so it can be propogated again.
	void	reset(void);
		
	// Give the photon a probabilistic chance of surviving or terminating
	// if its weight has dropped below a specified threshold.
	void	performRoulette(void);

	// Return the cartesian coordinates
	double	getX(void) {return location.x;}
	double	getY(void) {return location.y;}
	double	getZ(void) {return location.z;}

	// Return the direction cosines
	double	getDirX(void) {return direction.x;}
	double	getDirY(void) {return direction.y;}
	double	getDirZ(void) {return direction.z;}

	// Return the current weight of the photon
	double	getWeight(void) {return weight;}
	
	// Returns a random number 'n': 0 < n < 1
	double	getRandNum(void);
	
	// Return the calculated reflectance.
	double	getLayerReflectance(void);

	// Return the calculated medium reflectance (the boundary of the tissue).
	double	getMediumReflectance(void);

	// Return the status of the photon.
	bool	isAlive(void) {return status;}

	// Terminate the current photon.
	void	kill(void) {status = DEAD;}

	// Calculate the new location of the photon and
	// move it to those coordinates.
	void	updateLocation(void);

	// Update weight based on specular reflectance.
	void	specularReflectance(double n1, double n2);
	
	// Update the direction cosine when internal reflection occurs on z-axis.
	void	internallyReflectZ(void) {direction.z = -1*direction.z;}

	// Update the direction cosine when internal reflection occurs on y-axis.
	void	internallyReflectY(void) {direction.y = -1*direction.y;}
    
	// Update the direction cosine when internal reflection occurs on z-axis.
	void	internallyReflectX(void) {direction.x = -1*direction.x;}
    
	// Transmit the photon.
	void	transmit(const char *type);

	// Plot the photon's path.
	void	plotPath(void);
	
	// Inject the photon into the medium the given number of iterations.
	// 'state[1,2,3,4]' represent the random initial values for the state
	// of the random number generator.
	void	injectPhoton(Medium *m, const int num_iterations, unsigned int state1, unsigned int state2,
							unsigned int state3, unsigned int state4, coords &c);
    
    
    // Hop, Drop, Spin, Roulette and everything in between.
    // NOTE: 'iterations' are the number of photons simulated by this 'Photon' object.
    void    propagatePhoton(const int iterations);
	
	// Sets initial trajectory values.
	void	initTrajectory(void);
	
	// Zero's out the local detection array.
	void	initDetectionArray(void);

	// Initialize the RNG.
	void	initRNG(unsigned int s1, unsigned int s2, unsigned int s3, unsigned int s4);

	// Routines related to the thread-safe RNG
	unsigned int TausStep(unsigned int &z, int s1, int s2, int s3, unsigned long M);
	unsigned int LCGStep(unsigned int &z, unsigned int A, unsigned long C);
	double	HybridTaus(void);


	// Check if photon has come into contact with a layer boundary.
	bool 	hitLayerBoundary(void);

	// Check if photon has left the bounds of the medium.
	bool	hitMediumBoundary(void);
    
    
    // Store the energy lost into a local array that will be written to a global array
    // for all photons once they are DEAD.
    // This relieves contention between threads trying to update a single global data
    // structure and improves speed.
    void    updateLocalWeightArray(const double absorbed);


	
	
private:
	// Number of times this Photon (i.e., thread) will execute; where one execution
	// is the full cycle of photon propagation.
	int iterations;
	
	// Holds value of number of iterations thus far.
	int cnt;	
	
	// Structure that holds the location of the photon.
    coords location;
	
	// Radial position.
	double r;
	
	// Structure that holds the direction cosines of the photon.
    directionCos   direction;
	
	// Weight of the photon.
	double	weight;
	
	// Step size for the photon.
	double	step;
	
	// Step size to boundary.  Used when calculating distance from layer
	// boundary to current position of the photon.  Specifically, it is
	// the remainder of the step size after calculating the distance to
	// the layer boundary.
	// i.e. (step_size - distance_to_boundary)/mu_t
	double step_remainder;

	// status for current photon - dead (false) or alive (true).
	bool	status;
	
	// cosine and sine theta.  Used for trajectory calculations.
	double	cos_theta, sin_theta;
	
	// The azimuthal angle
	double	psi;
	
	// The value of internal reflectance that is compared to a random
	// number (uniform between (0,1]) to determine if the photon should
	// be transmitted or reflected on a stochastic basis.
	double	reflectance;
    
    // Transmission angle for a photon when it hits a layer boundary.
    double transmission_angle;

	
	// The number of steps this photon has taken while propagating through
	// the medium.
	int num_steps;
	
	// Pointer to the medium which this photon will propagate through.
	Medium *m_medium;

	// The thread id associated with this photon object.  The value is passed
	// in from the loop that creates the threads in main.cpp.
	int thread_id;

	// Local absorption array that holds values during execution.  This array
	// is copied over to the global absorption array (i.e. in the medium) once
	// a photon has finished propagating in the medium.
	double local_Cplanar[MAX_BINS];
	double	radial_size;			// Maximum radial size.
	int		num_radial_pos;			// Number of radial positions (i.e. NR).
	double	radial_bin_size;		// Radial bin size of the medium (i.e dr).


	// Boost Random Number Library implementation of Mersenne-twister RNG.
	//boost::mt19937 gen;

	// Used with the thread safe RNG to track state.
	unsigned int z1, z2, z3, z4;

	// Tracks whether or not a photon has hit a medium boundary.
	bool hit_x_bound, hit_y_bound, hit_z_bound;
    
    
    // Pointer to the current layer the photon is in.
    Layer *currLayer;
    
    // Structure that contains the cartesian coordinates of the injection point of each
    // photon into the medium.
    coords illuminationCoords;

}; 		

#endif // end PHOTON_H

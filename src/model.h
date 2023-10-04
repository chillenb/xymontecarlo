#include <vector>
#include <cmath>
#include <iostream>
#include <random>


#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#define K 64

namespace py = pybind11;
using std::vector;

typedef float Real;

class Model {
    public:
    Real __attribute__ ((__aligned__(32)))* spins;
    Real __attribute__ ((__aligned__(32)))* spins2;
    const uint32_t nx = K;
    const uint32_t ny = K;
    Real coupling;
    Real approx_angle = (Real) 0.5;
    Real ext_field;
    Real last_energy = 0.0;
    Real last_acceptance = 1.0;
    Real anglescale = 1.0;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis01;
    std::uniform_real_distribution<> dis11;
    std::uniform_real_distribution<> dist_ang;

    
    Model(Real coupling, Real ext_field) {
        spins = (Real*) aligned_alloc(32, nx*ny*sizeof(Real));
        spins2 = (Real*) aligned_alloc(32, nx*ny*sizeof(Real));
        this->coupling = coupling;
        this->ext_field = ext_field;
        this->gen = std::mt19937(rd());
        dis01 = std::uniform_real_distribution<>(0.0, 1.0);
        dis11 = std::uniform_real_distribution<>(-1.0, 1.0);
        dist_ang = std::uniform_real_distribution<>(-M_PI, M_PI);
        for(uint32_t i = 0; i < nx; i++) {
            for(uint32_t j = 0; j < ny; j++) {
                spins[nx*i + j] = dist_ang(gen);
            }
        }
    }


    ~Model() {
        if (spins != NULL) {
            free(spins);
            spins = NULL;
        }
        if (spins2 != NULL) {
            free(spins2);
            spins = NULL;
        }
    }


    double makepass(Real beta) {
        const uint32_t nx = K;
        const uint32_t ny = K;
        Real avg_acceptance = 0.0;
        Real energy = 0.0;
        Real angle;
        if (last_energy != 0.0) {
            if (last_acceptance >= 0.5) {
                anglescale *= (1+last_acceptance-0.5);
            } else {
                anglescale *= (1-0.5+last_acceptance);
            }
            anglescale = std::min(anglescale, (Real) 1.0);
            angle = anglescale;
        } else {
            angle = anglescale;
        }
        {
            for(uint32_t i = 0; i < K; i++) {
                for(uint32_t j = 0; j < K; j++) {
                    Real s = spins[K*i + j];
                    Real s1 = spins[K*i + ((j + 1) & (K-1))];
                    Real s2 = spins[K*((i + 1) & (K-1)) + j];
                    Real s3 = spins[K*i + (j - 1 + ny) & (K-1)];
                    Real s4 = spins[K*((i - 1 + K) & (K-1)) + j];
                    Real old_E = -coupling*(cos(s - s1) + cos(s - s2) + cos(s - s3) + cos(s - s4)) + ext_field*cos(s);
                    Real new_s = (s + angle*dist_ang(gen));
                    //new_s = fmod(new_s, M_PI);
                    Real new_E = -coupling*(cos(new_s - s1) + cos(new_s - s2) + cos(new_s - s3) + cos(new_s - s4)) + ext_field*cos(new_s);
                    Real deltaE = new_E - old_E;
                    Real acceptance = std::min((Real) 1.0, exp(-beta*deltaE));
                    avg_acceptance += acceptance;
                    spins[nx*i + j] = (dis01(gen) < acceptance) ? new_s : s;
                }
        }

        
        
        Real __attribute__ ((__aligned__(32)))* temp;
        //temp = spins;
        //spins = spins2;
        //spins2 = temp;
        for(uint32_t i = 0; i < nx; i++) {
            for(uint32_t j = 0; j < ny; j++) {
                Real s = spins[K*i + j];
                Real s1 = spins[K*i + ((j + 1) & (K-1))];
                Real s2 = spins[K*((i + 1) & (K-1)) + j];
                Real s3 = spins[K*i + (j - 1 + ny) & (K-1)];
                Real s4 = spins[K*((i - 1 + K) & (K-1)) + j];
                energy += -coupling*(cos(s - s1) + cos(s - s2) + cos(s - s3) + cos(s - s4)) + ext_field*cos(s);
            }
        }
        }
        //std::cout << "Average acceptance: " << avg_acceptance/(K*K) << std::endl;
        last_acceptance = avg_acceptance/(K*K);
        last_energy = energy;
        return energy;
    }


    std::vector<Real> makepasses(Real beta, uint32_t passes) {
        std::vector<Real> energies;
        for(uint32_t i = 0; i < passes; i++) {
            energies.push_back(makepass(beta));
        }
        return energies;
    }
};
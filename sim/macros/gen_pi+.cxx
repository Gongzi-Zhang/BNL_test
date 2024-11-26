#include "HepMC3/GenEvent.h"
#include "HepMC3/ReaderAscii.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/Print.h"

#include "TRandom3.h"
#include "TLorentzVector.h"

#include <iostream>
#include <cmath>
#include <math.h>
#include <TMath.h>

using namespace std;
using namespace HepMC3;

//Generate single neutral pions and decay to two photons.
void gen_pi0(int nevents = 1000, double theta_min = 0, double theta_max = 0.1, const char* out_fname = "gen_pi0.hepmc")
{
  gROOT->SetBatch(1);
  GenEvent evt(Units::GEV, Units::MM);
  double GeV = 1;
  double MeV = 1e-3*GeV;
  double mm = 1;
  double cm = 10*mm;

  //Pi0 mass
  double pi0_m = 0.13498*GeV;

  //Pi0 kinematics
  double eta_min = 4;
  double eta_max = 20;

  WriterAscii hepmc_output(out_fname);

  // Random number generator
  TRandom *r1 = new TRandom3(0);

  map<string, TH1F*> h1;
  h1["theta"] = new TH1F("theta", "opening angle between the two photons;rad", 100, 0, 0.02);
  h1["lt"] = new TH1F("lt", "transverse distance in the prototype;cm", 40, 0, 20);
  h1["E1"] = new TH1F("E1", "higher energy;GeV", 20, 20, 40);
  h1["E2"] = new TH1F("E2", "lower energy;GeV", 20, 0, 20);

  int ntotal = 0;
  int ei = 0;
  while (ei < nevents) {
    // Set pi0 momentum
    double E     = 40*GeV;
    double p     = sqrt(E*E - pi0_m*pi0_m);
    double phi   = r1->Uniform(0.0, 2.0 * M_PI);
    double eta   = r1->Uniform(eta_min, eta_max);
    double th    = 2.*atan(exp(-eta));
    double px    = p * cos(phi) * sin(th);
    double py    = p * sin(phi) * sin(th);
    double pz    = p * cos(th);

    TLorentzVector pi0_lab(px, py, pz, E);
    TVector3 boost_vec = pi0_lab.BoostVector();

    // Generate gammas in pi0 rest frame
    TLorentzVector g1_rest, g2_rest;

    double cost_g1rest = r1->Uniform(-1,1);
    double th_g1rest = acos(cost_g1rest);
    double sint_g1rest = sin(th_g1rest);

    double phi_g1rest = r1->Uniform(-1.*TMath::Pi(),1.*TMath::Pi());
    double cosp_g1rest = cos(phi_g1rest);
    double sinp_g1rest = TMath::Sin(phi_g1rest);

    g1_rest.SetE(pi0_m/2.);
    g1_rest.SetPx( (pi0_m/2.)*sint_g1rest*cosp_g1rest );
    g1_rest.SetPy( (pi0_m/2.)*sint_g1rest*sinp_g1rest );
    g1_rest.SetPz( (pi0_m/2.)*cost_g1rest );

    g2_rest.SetE(pi0_m/2.);
    g2_rest.SetPx( -g1_rest.Px() );
    g2_rest.SetPy( -g1_rest.Py() );
    g2_rest.SetPz( -g1_rest.Pz() );

    //Boost gammas to lab frame
    TLorentzVector g1_lab, g2_lab;

    g1_lab = g1_rest; g1_lab.Boost(boost_vec);
    g2_lab = g2_rest; g2_lab.Boost(boost_vec);

    // Filter: make sure the 2 photons will hit the prototype
    double z0 = 700*cm;
    double g1_x = g1_lab.Px()/g1_lab.Pz()*z0;
    double g1_y = g1_lab.Py()/g1_lab.Pz()*z0;
    double g2_x = g2_lab.Px()/g2_lab.Pz()*z0;
    double g2_y = g2_lab.Py()/g2_lab.Pz()*z0;
    double theta =  g1_lab.Angle(g2_lab.Vect());
    double lt = sqrt((g1_x - g2_x)*(g1_x - g2_x) + (g1_y - g2_y)*(g1_y - g2_y));

    double E1 = g1_lab.E();
    double E2 = g2_lab.E();
    if (E1 < E2)
    {
	E1 = g2_lab.E();
	E2 = g1_lab.E();
    }

    ntotal++;
    if (   theta < theta_min || theta > theta_max
        || g1_x < -9*cm || 9*cm < g1_x 
        || g1_y < -9*cm || 9*cm < g1_y 
        || g2_x < -9*cm || 9*cm < g2_x 
        || g2_y < -9*cm || 9*cm < g2_y )
        continue;

    h1["E1"]->Fill(E1/GeV);
    h1["E2"]->Fill(E2/GeV);
    h1["theta"]->Fill(theta);
    h1["lt"]->Fill(lt/cm);

    //Set the event number
    evt.set_event_number(ei);

    // FourVector(px,py,pz,e,pdgid,status)
    // type 4 is beam
    // pdgid 11 - electron
    // pdgid 2212 - proton
    GenParticlePtr p1 =
        make_shared<GenParticle>(FourVector(0.0, 0.0, 10.0, 10.0), 11, 4);
    GenParticlePtr p2 = 
	make_shared<GenParticle>(FourVector(0.0, 0.0, 0.0, 0.938), 2212, 4);

    // type 1 is final state
    // pdgid 22 - gamma
    GenParticlePtr p3 = make_shared<GenParticle>(
        FourVector(g1_lab.Px(), g1_lab.Py(), g1_lab.Pz(), g1_lab.E()),
	22, 1 );

    GenParticlePtr p4 = make_shared<GenParticle>(
        FourVector(g2_lab.Px(), g2_lab.Py(), g2_lab.Pz(), g2_lab.E()),
	22, 1 );

    GenVertexPtr v1 = make_shared<GenVertex>();
    v1->add_particle_in(p1);
    v1->add_particle_in(p2);

    v1->add_particle_out(p3);
    v1->add_particle_out(p4);
    evt.add_vertex(v1);

    if (ei == 0) {
      cout << "First event: " << endl;
      Print::listing(evt);
    }

    hepmc_output.write_event(evt);
    if (ei % 10000 == 0) {
      cout << "Event: " << ei << endl;
    }
    evt.clear();

    ei++;
  }

  hepmc_output.close();
  cout << "Events parsed and written: " << ei << endl;

  TCanvas c("c", "c", 800, 600);
  c.cd();
  for (auto& h : h1)
  {
    h.second->Draw();
    c.SaveAs(Form("%s.png", h.first.c_str()));
  }

  cout << "Event generation efficiency: " << nevents << "/" << ntotal 
       << "=" << nevents*100./ntotal << "%" << endl;
}

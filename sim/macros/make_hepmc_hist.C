#include <HepMC3/ReaderAscii.h>
#include <HepMC3/GenEvent.h>
#include <HepMC3/GenParticle.h>
#include <HepMC3/GenVertex.h>
#include <TH1D.h>
#include <TFile.h>
#include <TCanvas.h>
#include <iostream>
#include "calisim.h"

void make_hepmc_hist(const char* input, TString outFile="") {
    // Open HepMC3 file
    HepMC3::ReaderAscii reader(input);

    // units
    // const double mm = 1;
    // const double cm = 10*mm;
    // const double GeV = 1;

    TString hepmcFile(input);
    if (outFile.IsNull())
    {
	outFile = hepmcFile.ReplaceAll(".hepmc", "_hist.root");
    }
    TFile *output = new TFile(outFile, "RECREATE");
    output->cd();
    // Histograms 
    map<string, TH1F*> h1;
    h1["mul"] = new TH1F("mul", "multiplicity", 10, 0, 10);
    h1["x"] = new TH1F("x", "Vertex X;cm", 100, -0.5, 0.5);
    h1["y"] = new TH1F("y", "Vertex Y;cm", 100, -0.5, 0.5);
    h1["z"] = new TH1F("z", "Vertex Z;cm", 100, -0.5, 0.5);
    h1["calix"] = new TH1F("calix", "CALI X;cm", 100, cali_xmin/cm-5, cali_xmax/cm+5);
    h1["caliy"] = new TH1F("caliy", "CALI Y;cm", 100, cali_ymin/cm-5, cali_ymax/cm+5);
    h1["caliz"] = new TH1F("caliz", "CALI Z;cm", 100, cali_z0/cm-5, cali_zmax/cm+5);
    h1["p"] = new TH1F("p", "Momentum;GeV", 100, 0, 30);
    h1["eta"] = new TH1F("eta", "#eta", 100, cali_etamin-0.2, cali_etamax+0.2);
    h1["e"] = new TH1F("e", "Energy;GeV", 100, 0, 30);
    h1["m"] = new TH1F("m", "Mass;GeV", 100, 0, 1);
    h1["pi_e"] = new TH1F("pi_e", "#pi^{#pm} Energy;GeV", 100, 0, 50);
    h1["gamma_e"] = new TH1F("gamma_e", "#gamma Energy;GeV", 100, 0, 50);
    h1["pid"] = new TH1F("pid", "Particle ID", 13, 0.5, 13.5);
    h1["pid"]->GetXaxis()->SetBinLabel(1, "e^{#pm}");
    h1["pid"]->GetXaxis()->SetBinLabel(2, "#gamma");
    h1["pid"]->GetXaxis()->SetBinLabel(3, "#pi^{0}");
    h1["pid"]->GetXaxis()->SetBinLabel(4, "#pi^{+}");
    h1["pid"]->GetXaxis()->SetBinLabel(5, "#pi^{-}");
    h1["pid"]->GetXaxis()->SetBinLabel(6, "K^{0}");
    h1["pid"]->GetXaxis()->SetBinLabel(7, "K^{+}");
    h1["pid"]->GetXaxis()->SetBinLabel(8, "K^{-}");
    h1["pid"]->GetXaxis()->SetBinLabel(9, "P^{+}");
    h1["pid"]->GetXaxis()->SetBinLabel(10, "P^{-}");
    h1["pid"]->GetXaxis()->SetBinLabel(11, "N");
    h1["pid"]->GetXaxis()->SetBinLabel(12, "Nbar");
    h1["pid"]->GetXaxis()->SetBinLabel(13, "others");

    // Event loop
    int count;
    int pid;
    double x0, y0, z0;
    double x1, y1, z1;
    double px, py, pz, p, m, e;
    double eta;
    HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);
    while (!reader.failed()) {
	reader.read_event(evt);
	if (evt.particles().empty()) continue;

	count = 0;
	// Loop over particles in the event
	for (auto particle : evt.particles()) {
	    if (!particle) continue;
	    if (particle->status() != 1) continue;

	    pid = particle->pid();
	    HepMC3::GenVertexPtr vertex = particle->production_vertex();
	    x0 = vertex->position().x();
	    y0 = vertex->position().y();
	    z0 = vertex->position().z();
	    HepMC3::FourVector momentum = particle->momentum();
	    px = momentum.px();
	    py = momentum.py();
	    pz = momentum.pz();
	    p  = sqrt(px*px + py*py + pz*pz);
	    eta = 0.5*log((p+pz)/(p-pz));
	    e  = momentum.e();
	    m  = sqrt(e*e - p*p);

	    // check if the particle will reach our CALI prototype
	    if (e > 1)  // require particle energy to be larger than 1 GeV
	    {
		bool hit = false;
		// the front face
		x1 = x0 + (cali_z0 - z0)*px/pz;
		y1 = y0 + (cali_z0 - z0)*py/pz;
		if (cali_xmin < x1 && x1 < cali_xmax 
		 && cali_ymin < y1 && y1 < cali_ymax)	
		{ 
		    hit = true; 
		    z1 = cali_z0;
		}
		else
		{   // the right face
		    y1 = y0 + (cali_xmin - x0)*py/px;
		    z1 = z0 + (cali_xmin - x0)*pz/px;
		    if (cali_ymin < y1 && y1 < cali_ymax 
		     && cali_z0   < z1 && z1 < cali_zmax)	
		    {
			x1 = cali_xmin;
			hit = true;
		    }
		}

		if (hit)
		{
		    count++;

		    h1["x"]->Fill(x0/cm);
		    h1["y"]->Fill(y0/cm);
		    h1["z"]->Fill(z0/cm);
		    h1["calix"]->Fill(x1/cm);
		    h1["caliy"]->Fill(y1/cm);
		    h1["caliz"]->Fill(z1/cm);
		    h1["p"]->Fill(p/GeV);
		    h1["eta"]->Fill(eta);
		    h1["e"]->Fill(e/GeV);
		    h1["m"]->Fill(sqrt(e*e - p*p)/GeV);

		    if (pid == 11 || pid == -11)
			h1["pid"]->Fill(1);
		    else if (pid == 22)
		    {
			h1["pid"]->Fill(2);
			h1["gamma_e"]->Fill(e/GeV);
		    }
		    else if (pid == 111)
			h1["pid"]->Fill(3);
		    else if (pid == 211)
		    {
			h1["pid"]->Fill(4);
			h1["pi_e"]->Fill(e/GeV);
		    }
		    else if (pid == -211)
		    {
			h1["pid"]->Fill(5);
			h1["pi_e"]->Fill(e/GeV);
		    }
		    else if (pid == 130)
			h1["pid"]->Fill(6);
		    else if (pid == 321)
			h1["pid"]->Fill(7);
		    else if (pid == -321)
			h1["pid"]->Fill(8);
		    else if (pid == 2212)
			h1["pid"]->Fill(9);
		    else if (pid == -2212)
			h1["pid"]->Fill(10);
		    else if (pid == 2112)
			h1["pid"]->Fill(11);
		    else if (pid == -2112)
			h1["pid"]->Fill(12);
		    else
		    {
			cout << pid << endl;
			h1["pid"]->Fill(13);
		    }
		    break;
		}
	    }
	}
	h1["mul"]->Fill(count);
    }
    h1["pid"]->Scale(1/h1["pid"]->Integral());

    // Save histograms to a ROOT file
    output->Write();
    output->Close();
}

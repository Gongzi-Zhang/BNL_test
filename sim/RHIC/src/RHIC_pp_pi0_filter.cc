// Adapted from main42.cc example code

// Input and output files are specified on the command line, e.g. like
// ./RHIC_pp RHIC_pp.cmnd RHIC_pp.hepmc | tee stdout.dat
// The main program contains no analysis; this is intended to happen later.
// It therefore "never" has to be recompiled to handle different tasks.

#include <stdlib.h>
#include <unistd.h>
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC3.h"

using namespace std;
using namespace Pythia8;

int main(int argc, char* argv[]) {

  // Check that correct number of command-line arguments
  if (argc != 3) {
    cerr << "FATAL\t"
	 << "\tUnexpected number of command-line arguments.\n"
	 << "\tYou are expected to provide one input and one output file name. \n"
         << "\tProgram stopped! " << endl;
    return 1;
  }

  // Check that the provided input name corresponds to an existing file.
  ifstream is(argv[1]);
  if (!is) {
    cerr << "ERROR\t"
	 << "\tCommand-line file " << argv[1] << " was not found. \n"
         << "\tProgram stopped! " << endl;
    return 1;
  }

  // Confirm that external files will be used for input and output.
  cout << "INFO\tPYTHIA settings will be read from file " << argv[1] << endl
       << "\tHepMC events will be written to file " << argv[2] << endl;

  // Interface for conversion from Pythia8::Event to HepMC event.
  HepMC3::Pythia8ToHepMC3 toHepMC;

  // Specify file where HepMC events will be stored.
  HepMC3::WriterAscii ascii_io(argv[2]);

  // Generator.
  Pythia pythia;

  // Read in commands from external file.
  pythia.readFile(argv[1]);

  // Extract settings to be used in the main program.
  int    nEvent    = pythia.mode("Main:numberOfEvents");
  int    nAbort    = pythia.mode("Main:timesAllowErrors");

  srand(time(0));
  int rnd = rand();
  char setRandom[100];
  sprintf(setRandom, "Random:seed = %d", rnd + getpid());
  pythia.readString("Random:setSeed = on");
  pythia.readString(setRandom);

  // Initialization.
  pythia.init();

  // pythia units
  double mm = 1;
  double cm = 10*mm;

  // CALI prototype geometry
  double cali_x0 = 50*cm;
  double cali_y0 = 0;
  double cali_xw = 95.2*mm;
  double cali_yw = cali_xw;
  double cali_xmin = cali_x0 - cali_xw/2;
  double cali_xmax = cali_x0 + cali_xw/2;
  double cali_ymin = cali_y0 - cali_yw/2;
  double cali_ymax = cali_y0 + cali_yw/2;
  double cali_z0 = 700*cm;
  double cali_lt = 24.5*mm;	 // layer thickness
  int	 cali_nLayers = 12;
  double cali_z1 = cali_z0 + cali_lt*cali_nLayers/2;
  double cali_z2 = cali_z0 + cali_lt*cali_nLayers;
  double cali_z[] = {cali_z0, cali_z1, cali_z2};

  // Begin event loop.
  int iAbort = 0;
  int nTotal = 0;
  int iEvent = 0;
  while (iEvent < nEvent) {

    // Generate event.
    if (!pythia.next()) {

      // If failure because reached end of file then exit event loop.
      if (pythia.info.atEndOfFile()) {
        cout << "ERROR\tAborted since reached end of Les Houches Event File\n";
        break;
      }

      // First few failures write off as "acceptable" errors, then quit.
      if (++iAbort < nAbort) continue;
      cout << "ERROR\tEvent generation aborted prematurely, owing to error!\n";
      break;
    }
    nTotal++;

    // pi0 filter
    double x0, y0, z0;
    double x, y;
    double px, py, pz;
    bool keepit = false;
    for (int i=0; i<pythia.event.size(); i++)
    {
	Particle& p = pythia.event[i];
	
	// if (!p.isFinal())	// pi0 is not the final state particle
	//     continue;

	if (p.id() != 111)
	    continue;

	x0 = p.xProd();
	y0 = p.yProd();
	z0 = p.zProd();
	px = p.px();
	py = p.py();
	pz = p.pz();

	// check if the pi0 will reach our CALI prototype
	for (int zi=0; zi<3; zi++)
	{
	    x = x0 + (cali_z[zi] - z0)*px/pz;
	    y = y0 + (cali_z[zi] - z0)*py/pz;
	    if (cali_xmin < x && x < cali_xmax 
	     && cali_ymin < y && y < cali_ymax)
	    {
		keepit = true;
		break;
	    }
	}

	if (keepit)
	    break;
    }

    if (!keepit)
	continue;

    iEvent++;
    // Construct new empty HepMC event and fill it.
    // Default units are ( HepMC3::Units::GEV, HepMC3::Units::MM)
    // but can be changed in the GenEvent constructor.
    HepMC3::GenEvent hepmcevt;
    toHepMC.fill_next_event( pythia, &hepmcevt );

    // Write the HepMC event to file.
    ascii_io.write_event(hepmcevt);

  // End of event loop. Statistics.
  }
  pythia.stat();
  cout << "INFO\t" << nEvent << " / " << nTotal << " = " << nEvent*1./nTotal << " events produced." << endl;

  // Done.
  return 0;
}

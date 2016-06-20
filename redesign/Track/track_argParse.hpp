#ifndef _TRACK_ARGPARSE_HPP_
#define _TRACK_ARGPARSE_HPP_

#include "afx_track.hpp"

namespace Parser {
	struct ArgParser_Track : public AFX::Parser::ArgParser {
	public:
		typedef AFX::Parser::ArgParser::ArgNo ArgNo;
		typedef AFX::Parser::ArgParser::Args Args;
		explicit ArgParser_Track ();
		~ArgParser_Track ();
		inline void operator () (ArgNo, Args const []) final;
		char * m_videoName;
	protected:
		inline void usage () final;
		inline void errorHandler (int) final;
	private:
		char * m_executable;
	};
}

namespace Parser {
	ArgParser_Track::ArgParser_Track () {}

	ArgParser_Track::~ArgParser_Track () {}

	void ArgParser_Track::operator () (ArgParser_Track::ArgNo argc, ArgParser_Track::Args const argv[]) {
		int c = -1;
		this->m_executable = basename(argv[0]);
		this->m_videoName = nullptr;
		while((c = getopt (argc, argv, "hS:E:L:W:N:d:en:pv:s:x:y:t:a:")) != -1) {
			switch(c) {
				case 'S':
					Attributes::Tracking::start_frame = atoi (optarg);
				break;
				case 'E':
					Attributes::Tracking::end_frame = atoi (optarg);
				break;
				case 'L':
					Attributes::Tracking::traj_length = atoi (optarg);
				break;
				case 'W':
					Attributes::Tracking::min_distance = atoi (optarg);
				break;
				case 'N':
					Attributes::Descriptor::Cube::patch_size = atoi (optarg);
				break;
				case 'd':
					Attributes::Saliency::dynamic_ratio = atof (optarg);
				break;
				case 'e':
					Attributes::Mode::toDisplay = true;
				break;
				case 'p':
					Attributes::Mode::toPreserve = true;
				break;
				case 's':
					Attributes::Saliency::static_ratio = atof (optarg);
				break;
				case 'x':
					Attributes::Descriptor::Cube::nxCells = atoi (optarg);
				break;
				case 'y':
					Attributes::Descriptor::Cube::nyCells = atoi (optarg);
				break;
				case 't':
					Attributes::Descriptor::Cube::ntCells = atoi (optarg);
				break;
				case 'n':
					Attributes::Descriptor::MultiScale::scale_num = atoi (optarg);
				break;
				case 'v':
					this->m_videoName = optarg;
				break;
				case 'o':
					Attributes::Descriptor::MultiScale::scale_num = atoi (optarg);
				break;
				case 'h':
					this->usage ();
					exit (0);
				break;
				case 'a':
					Attributes::Saliency::alpha = atof (optarg);
					break;
				default:
					this->errorHandler (c);
			}
		}
	}

	void ArgParser_Track::usage () {
		std::cerr << "Extract dense trajectories from a video\n\n";
		std::cerr << "Usage: Track -v video_file [options]\n";
		std::cerr << "Options:\n";
		std::cerr << "  -h                        Display this message and exit\n";
		std::cerr << "  -S [start frame]          The start frame to compute feature (default: S=0 frame)\n";
		std::cerr << "  -E [end frame]            The end frame for feature computing (default: E=last frame)\n";
		std::cerr << "  -L [trajectory length]    The length of the trajectory (default: L=15 frames)\n";
		std::cerr << "  -W [sampling stride]      The stride for dense sampling feature points (default: W=5 pixels)\n";
		std::cerr << "  -N [neighborhood size]    The neighborhood size for computing the descriptor (default: N=32 pixels)\n";
		std::cerr << "  -d [dynamic ratio]        set dynamic saliency ratio\n";
		std::cerr << "  -e [display toggler]      Enable display mode\n";
		std::cerr << "  -n [maximum scale num]    The maximum scale number(default:8) of the image pyramid\n";
		std::cerr << "  -p [preserve toggler]     Enable preserve mode\n";
		std::cerr << "  -s [static ratio]         set static saliency ratio\n";
		std::cerr << "  -v [video name]           The name of the video to process\n";
		std::cerr << "  -x [spatial x cells]      The number of cells in the nx axis (default: nx=2 cells)\n";
		std::cerr << "  -y [spatial y cells]      The number of cells in the ny axis (default: ny=2 cells)\n";
		std::cerr << "  -t [temporal cells]       The number of cells in the nt axis (default: nt=3 cells)\n";
		std::cerr << "  -a [pooling balance]      The balance factor between max pooling and average pooling\n";
	}

	void ArgParser_Track::errorHandler (int opt) {
		std::cerr << "error parsing arguments at -"<< opt << "\n  Try '" << this->m_executable << " -h' for help." << std::endl;
		abort();
	}
}

#endif// ! _TRACK_ARGPARSE_HPP_

#ifndef _CLUSTER_ATTRIBUTES_HPP_
#define _CLUSTER_ATTRIBUTES_HPP_

namespace Attributes {
    namespace Dimension {
        static const        int     hog         =   96;
        static const        int     hof         =   108;
        static const        int     mbh         =   192;
    }

    namespace SelectFeatures {
        static const        int     min_line    =   20;
        static const        double  ratio       =   0.0635;
    }

    namespace Cluster {
        static const        int     K           =   4000;
        static const        int     stages      =   100;
    }
}

#endif// ! _CLUSTER_ATTRIBUTES_HPP_

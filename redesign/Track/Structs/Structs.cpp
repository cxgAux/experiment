#include "Structs.hpp"

namespace Structs {
    TrackerInfo::TrackerInfo (const int trajLength, const int initGap) : m_iTrajLength(trajLength), m_iInitGap(initGap) {}

    TrackerInfo::~TrackerInfo() {}
}

namespace Structs {
    DescInfo::CubeInfo::CubeInfo (const int xCells, const int yCells, const int tCells, const int width, const int height)
        : m_iXCells(xCells), m_iYCells(yCells), m_iTCells(tCells), m_iBlockWidth(width), m_iBlockHeight(height) {}

    DescInfo::CubeInfo::CubeInfo (const CubeInfo & cubeInfo)
        : CubeInfo(cubeInfo.m_iXCells, cubeInfo.m_iYCells, cubeInfo.m_iTCells, cubeInfo.m_iBlockWidth, cubeInfo.m_iBlockHeight) {}

    DescInfo::CubeInfo & DescInfo::CubeInfo::operator= (const CubeInfo & cubeInfo) {
        this->m_iXCells = cubeInfo.m_iXCells;
        this->m_iYCells = cubeInfo.m_iYCells;
        this->m_iTCells = cubeInfo.m_iTCells;
        this->m_iBlockWidth = cubeInfo.m_iBlockWidth;
        this->m_iBlockHeight = cubeInfo.m_iBlockHeight;
        return *this;
    }

    DescInfo::CubeInfo::~CubeInfo () {}

    DescInfo::DescInfo (const int xCells, const int yCells, const int tCells, const int width, const int height, const int bins, const int normType)
        : m_cubeInfo(xCells, yCells, tCells, width, height), m_iBin(bins), m_iNormType(normType) {}

    DescInfo::DescInfo (const DescInfo & descInfo)
        : m_cubeInfo(descInfo.m_cubeInfo), m_iBin(descInfo.m_iBin), m_iNormType(descInfo.m_iNormType) {}

    DescInfo & DescInfo::operator= (const DescInfo & descInfo) {
        this->m_cubeInfo = descInfo.m_cubeInfo;
        this->m_iBin = descInfo.m_iBin;
        this->m_iNormType = descInfo.m_iNormType;
        return *this;
    }

    DescInfo::~DescInfo () {}

    int DescInfo::getDim () const {
        return this->m_cubeInfo.m_iXCells * this->m_cubeInfo.m_iYCells * this->m_iBin;
    }
}

namespace Structs {
    DescMat::DescMat (const int width, const int height, const int bin)
        : m_iWidth(width), m_iHeight(height), m_iBin(bin) {}

    DescMat::~DescMat() {}
}

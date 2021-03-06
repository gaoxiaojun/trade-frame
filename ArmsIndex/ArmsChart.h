/************************************************************************
 * Copyright(c) 2013, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/
// Started 2013/12/01

#pragma once

#ifdef _M_X64
#include <OUCharting/ChartDirector64/chartdir.h>
#else
#include <OUCharting/ChartDirector/chartdir.h>
#endif

class ArmsChart {
public:
  ArmsChart( void );
  ~ArmsChart(void);
protected:
private:
  int m_nWidthX;
  int m_nWidthY;
};


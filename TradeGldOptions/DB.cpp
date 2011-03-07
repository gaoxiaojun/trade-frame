/************************************************************************
 * Copyright(c) 2011, One Unified. All rights reserved.                 *
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

#include "StdAfx.h"

#include <stdexcept>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include "DB.h"

using namespace ou::tf;

CDB::CDB(void): m_bOpened( false ) {
}

CDB::~CDB(void) {
  Close();
}

void CDB::Open( const std::string& sDbName ) {

  if ( !m_bOpened ) {
    if ( boost::filesystem::exists( sDbName ) ) {
      // open already created and loaded database
      m_session.Open( sDbName );
      ou::tf::db::RegisterRowDefinitions( m_session );
    }
    else {
      // create and build new database
      m_session.Open( sDbName, ou::db::EOpenFlagsAutoCreate );
      ou::tf::db::RegisterTableCreation( m_session );
      m_session.CreateTables();
      ou::tf::db::RegisterRowDefinitions( m_session );
      ou::tf::db::PopulateTables( m_session );
      Populate();
    }
    m_bOpened = true;
  }

}

void CDB::Close( void ) {
  if ( m_bOpened ) {
    m_bOpened = false;
    m_session.Close();
  }
}

void CDB::Populate( void ) {

  CAccountAdvisor::TableRowDef aa( "ray", "Raymond Burkholder", "One Unified" );
  ou::db::QueryFields<CAccountAdvisor::TableRowDef>::pQueryFields_t paa = m_session.Insert<CAccountAdvisor::TableRowDef>( aa );

  CAccountOwner::TableRowDef ao( "ray", "ray", "Raymond", "Burkholder" );
  ou::db::QueryFields<CAccountOwner::TableRowDef>::pQueryFields_t pao = m_session.Insert<CAccountOwner::TableRowDef>( ao );

  CAccount::TableRowDef acctIB( "ib01", "ray", "Raymond Burkholder", keytypes::EProviderIB, "Interactive Brokers", "acctid", "login", "password" );
  ou::db::QueryFields<CAccount::TableRowDef>::pQueryFields_t paIB = m_session.Insert<CAccount::TableRowDef>( acctIB );

  CAccount::TableRowDef acctIQ( "iq01", "ray", "Raymond Burkholder", keytypes::EProviderIQF, "IQFeed", "acctid", "login", "password" );
  ou::db::QueryFields<CAccount::TableRowDef>::pQueryFields_t paIQF = m_session.Insert<CAccount::TableRowDef>( acctIQ );

  CPortfolio::TableRowDef portfolio( PortfolioId(), "ray", "Delta Neutral 01 - long und, long put" );
  ou::db::QueryFields<CPortfolio::TableRowDef>::pQueryFields_t pPortfolio = m_session.Insert<CPortfolio::TableRowDef>( portfolio );

}

struct Query {
  template<class A>
  void Fields( A& a ) {
    ou::db::Field( a, "portfolioid", idPortfolio );
  }
  ou::tf::keytypes::idPortfolio_t idPortfolio;
  Query( void ) {};
  Query( ou::tf::keytypes::idPortfolio_t idPortfolio_ ) : idPortfolio( idPortfolio_ ) {};
};

void CDB::LoadPortfolio( const ou::tf::keytypes::idPortfolio_t& id, CPortfolio::pPortfolio_t& pPortfolio ) {

  Query query( PortfolioId() );
  
  CPortfolio::TableRowDef portfolio;
  ou::db::QueryFields<Query>::pQueryFields_t pQuery 
    = m_session.SQL<Query>( "select * from portfolios", query ).Where( "portfolioid = ?" ).NoExecute();

  m_session.Bind<Query>( pQuery );
  if ( m_session.Execute( pQuery ) ) {
    m_session.Columns<Query, CPortfolio::TableRowDef>( pQuery, portfolio );
  }
  else {
    throw std::runtime_error( "no portfolio found" );
  }

  pPortfolio.reset( new CPortfolio( portfolio ) );

}
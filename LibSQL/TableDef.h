/************************************************************************
 * Copyright(c) 2010, One Unified. All rights reserved.                 *
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

#pragma once

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "Functions.h"
#include "Sql.h"

namespace ou {
namespace db {

// Action_CreateTable

class Action_CreateTable {
public:

  Action_CreateTable( void );
  ~Action_CreateTable( void );

  void registerField( const std::string& sField, const char* szDbFieldType );
  void registerConstraint( const std::string& sLocalField, const std::string& sRemoteTable, const std::string& sRemoteField );
  void setIsKey( const std::string& sLocalField );

  void ComposeStatement( const std::string& sTableName, std::string& sStatement );

protected:

private:

  // definition of fields

  struct structFieldDef {
    std::string sFieldName;
    std::string sFieldType;
    bool bIsKeyPart;
    structFieldDef( void ): bIsKeyPart( false ) {};
    structFieldDef(const std::string& sFieldName_, const std::string& sFieldType_ ) 
      : bIsKeyPart( false ), sFieldName( sFieldName_ ), sFieldType( sFieldType_ ) {};
  };

  unsigned int m_cntKeys;

  typedef std::vector<structFieldDef> vFields_t;
  typedef vFields_t::iterator vFields_iter_t;
  vFields_t m_vFields;

  struct structConstraint {
    std::string sLocalField;
    std::string sRemoteTable;
    std::string sRemoteField;
    structConstraint( const std::string& sLocalField_, const std::string& sRemoteTable_, const std::string& sRemoteField_ ):
      sLocalField( sLocalField_ ), sRemoteTable( sRemoteTable_ ), sRemoteField( sRemoteField_ ) {};
  };

  typedef std::vector<structConstraint> vConstraints_t;
  typedef vConstraints_t::iterator vConstraints_iter_t;
  vConstraints_t m_vConstraints;

};

// TableDef_BuildStatement

// TableDef_BindForWrite

// TableDef_ColumnForRead

//
// CTableDef
//

template<class T>  // T: Table Class with TableDef member function
class CTableDef: public CSql<T> {
public:

  typedef boost::shared_ptr<CTableDef<T> > pCTableDef_t;

  CTableDef( IDatabase& db, const std::string& sTableName );
  ~CTableDef( void ) {};

  const std::string& TableName( void ) { return m_sTableName; };

protected:

  void ComposeStatement( std::string& sStatement );

private:
  std::string m_sTableName;
  CTableDef( void );  // no default constructor
};

template<class T>
CTableDef<T>::CTableDef( IDatabase& db, const std::string& sTableName )
: CSql<T>( db ), 
  m_sTableName( sTableName )
{
  PrepareStatement();
}

template<class T>
void CTableDef<T>::ComposeStatement( std::string& sStatement ) {

  Action_CreateTable ct;  // action structure maintenance

  T t;

  t.Fields( ct );  // build structure from source definitions

  ct.ComposeStatement( TableName(), sStatement );  // build statement from structures

}

} // db
} // ou
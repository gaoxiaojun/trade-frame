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

//#include <TFHDF5TimeSeries/HDF5DataManager.h>
//#include <TFHDF5TimeSeries/HDF5TimeSeriesContainer.h>

#include <TFTrading/InstrumentManager.h>

#include "Strategy.h"

Strategy::Strategy(void) 
  : m_sim( new ou::tf::CSimulationProvider() ),
  m_sma1min( &m_quotes, 60 ), m_sma2min( &m_quotes, 120 ), m_sma3min( &m_quotes, 180 ),
  m_sma5min( &m_quotes, 300 ), m_sma15min( &m_quotes, 1800 ),
//  m_stateTrade( ETradeOut ), m_dtEnd( date( 2011, 9, 23 ), time_duration( 17, 58, 0 ) ),
  m_stateTrade( ETradeOut ), m_dtEnd( date( 2011, 11, 8 ), time_duration( 17, 58, 0 ) ),  // put in time start
  m_nTransitions( 0 ),
  m_barFactory( 180 ),
  m_dvChart( "Strategy1", "GC" ), 
  m_ceShorts( ou::ChartEntryShape::ESell, ou::Colour::Red ),
  m_ceLongs( ou::ChartEntryShape::EBuy, ou::Colour::Green )
{

  ou::tf::CProviderManager::Instance().Register( "sim01", static_cast<pProvider_t>( m_sim ) );

  m_dvChart.Add( 0, m_ceBars );
  m_dvChart.Add( 0, m_ceSMA );
  m_dvChart.Add( 0, m_ceShorts );
  m_dvChart.Add( 0, m_ceLongs );
  m_dvChart.Add( 1, m_ceVolume );
  m_dvChart.Add( 2, m_ceSlope );
//  m_dvChart.Add( 3, m_ceRR );
  m_dvChart.Add( 4, m_cePLLongOpen );
  m_dvChart.Add( 4, m_cePLShortOpen );
  m_dvChart.Add( 4, m_cePLLongClose );
  m_dvChart.Add( 4, m_cePLShortClose );

  m_ceSMA.SetColour( ou::Colour::DarkOliveGreen );
  m_cePLLongOpen.SetColour( ou::Colour::Green );
  m_cePLShortOpen.SetColour( ou::Colour::Red );
  m_cePLLongClose.SetColour( ou::Colour::GreenYellow );
  m_cePLShortClose.SetColour( ou::Colour::RosyBrown );

  m_barFactory.SetOnBarComplete( MakeDelegate( this, &Strategy::HandleBarCompletion ) );

  m_sim->OnConnected.Add( MakeDelegate( this, &Strategy::HandleSimulatorConnected ) );
  m_sim->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleSimulatorDisConnected ) );

  m_pExecutionProvider = m_sim;
  m_pExecutionProvider->OnConnected.Add( MakeDelegate( this, &Strategy::HandleOnExecConnected ) );
  m_pExecutionProvider->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleOnExecDisconnected ) );

  m_pDataProvider = m_sim;
  m_pDataProvider->OnConnected.Add( MakeDelegate( this, &Strategy::HandleOnData1Connected ) );
  m_pDataProvider->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleOnData1Disconnected ) );

  ou::tf::CInstrumentManager& mgr( ou::tf::CInstrumentManager::Instance() );
  m_pTestInstrument = mgr.Exists( "+GCZ11" ) ? mgr.Get( "+GCZ11" ) : mgr.ConstructFuture( "+GCZ11", "SMART", 2011, 12 );
  m_pTestInstrument->SetMultiplier( 100 );

//  m_sim->SetGroupDirectory( "/semiauto/2011-Sep-23 19:17:48.252497" );
  //m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-06 18:54:22.184889" );
  m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-07 18:53:31.016760" );
  //m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-08 18:58:29.396624" );
  m_sim->SetExecuteAgainst( ou::tf::CSimulateOrderExecution::EAQuotes );
  
  m_sim->AddQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleQuote ) );
  m_sim->AddTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleTrade ) );
  m_sim->SetOnSimulationComplete( MakeDelegate( this, &Strategy::HandleSimulationComplete ) );

  m_pPositionLongOpen.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  m_pPositionLongOpen->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  m_pPositionLongOpen->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pPositionLongClose.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  //m_pPositionLongClose->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  //m_pPositionLongClose->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pOrdersOutstandingLongs = new OrdersOutstandingLongs( m_pPositionLongClose );

  m_pPositionShortOpen.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  m_pPositionShortOpen->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  m_pPositionShortOpen->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pPositionShortClose.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  //m_pPositionShortClose->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  //m_pPositionShortClose->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pOrdersOutstandingShorts = new OrdersOutstandingShorts( m_pPositionShortClose );
}

Strategy::~Strategy(void) {

  m_sim->SetOnSimulationComplete( 0 );
  m_sim->RemoveQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleQuote ) );
  m_sim->RemoveTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleTrade ) );

  m_pDataProvider->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleOnData1Connected ) );
  m_pDataProvider->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleOnData1Disconnected ) );

  m_pExecutionProvider->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleOnExecConnected ) );
  m_pExecutionProvider->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleOnExecDisconnected ) );

  m_sim->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleSimulatorConnected ) );
  m_sim->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleSimulatorDisConnected ) );

  m_barFactory.SetOnBarComplete( 0 );

  ou::tf::CProviderManager::Instance().Release( "sim01" );

}

void Strategy::HandleSimulatorConnected( int ) {
  m_sim->Run();
}

void Strategy::HandleSimulatorDisConnected( int ) {
}

void Strategy::HandleOnExecConnected( int ) {
}

void Strategy::HandleOnExecDisconnected( int ) {
}

void Strategy::HandleOnData1Connected( int ) {
}

void Strategy::HandleOnData1Disconnected( int ) {
}

void Strategy::Start( const std::string& sSymbolPath ) {
  m_sim->Connect();
}

void Strategy::HandleQuote( const ou::tf::CQuote& quote ) {

  if ( ( 0 == quote.Ask() ) || ( 0 == quote.Bid() ) || ( 0 == quote.AskSize() ) || ( 0 == quote.BidSize() ) ) {
    return;
  }

  // problems occur when long trend happens and can't get out of oposing position.

  m_quotes.Append( quote );
  m_sma5min.Update();

  if ( 500 < m_quotes.Size() ) {

    //m_pOrdersOutstandingLongs->HandleQuote( quote );
    //m_pOrdersOutstandingShorts->HandleQuote( quote );

    m_ceSlope.Add( quote.DateTime(), m_sma5min.Slope() );
    m_ceSMA.Add( quote.DateTime(), m_sma5min.MeanY() );
    //m_ceRR.Add( quote.DateTime(), m_sma5min.RR() );
    m_cePLLongOpen.Add( quote.DateTime(), m_pPositionLongOpen->GetRealizedPL() );
    m_cePLShortOpen.Add( quote.DateTime(), m_pPositionShortOpen->GetRealizedPL() );
//    m_cePLLongClose.Add( quote.DateTime(), m_pPositionLongOpen->GetUnRealizedPL() );
//    m_cePLShortClose.Add( quote.DateTime(), m_pPositionShortOpen->GetUnRealizedPL() );

    //m_cePLLongOpen.Add( quote.DateTime(), 
      //m_pPositionLongOpen->GetUnRealizedPL()
      //+ m_pPositionShortOpen->GetUnRealizedPL()
       //);

    switch ( m_stateTrade ) {
    case ETradeOut:
      if ( 0 != m_sma5min.Slope() ) {
        if ( 0 < m_sma5min.Slope() ) { //rising
          m_ss.str( "" );
          m_ss << "Ordered: " << quote.DateTime() << "; ";
          m_pOrder = m_pPositionLongOpen->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Buy, 1 );
          m_ceLongs.AddLabel( quote.DateTime(), quote.Ask(), "Long a" );
          m_stateTrade = ETradeLong;
        }
        else { // falling
          m_ss.str( "" );
          m_ss << "Ordered: " << quote.DateTime() << "; ";
          m_pOrder = m_pPositionShortOpen->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Sell, 1 );
          m_ceShorts.AddLabel( quote.DateTime(), quote.Bid(), "Short a" );
          m_stateTrade = ETradeShort;
        }
      }
      break;
    case ETradeLong:
      if ( m_pPositionLongOpen->OrdersPending() ) {
      //if ( false ) {
      }
      else {
        if ( quote.DateTime() > m_dtEnd ) {
          m_pOrder.reset();
          m_pOrdersOutstandingLongs->CancelAll();
          m_pOrdersOutstandingShorts->CancelAll();
          m_pPositionLongOpen->ClosePosition();
          m_pPositionShortOpen->ClosePosition();
          m_stateTrade = ETradeDone;
        }
        else {
          if ( 0 > m_sma5min.Slope() ) {
            //pPosition->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Sell, 1 );
            ++m_nTransitions;
            //m_ss.str( "" );
            //m_ss << "Ordered: " << quote.DateTime() << "; ";
            m_pPositionLongOpen->ClosePosition();
            //m_pOrdersOutstandingLongs->AddOrderFilling( m_pOrder );
            m_pOrder.reset();
            m_pOrder = m_pPositionShortOpen->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Sell, 1 );
            m_ceShorts.AddLabel( quote.DateTime(), quote.Bid(), "Short b" );
            m_stateTrade = ETradeShort;
          }
        }
      }
      break;
    case ETradeShort:
      if ( m_pPositionShortOpen->OrdersPending() ) {
      //if ( false ) {
      }
      else {
        if ( quote.DateTime() > m_dtEnd ) {
          m_pOrder.reset();
          m_pOrdersOutstandingLongs->CancelAll();
          m_pOrdersOutstandingShorts->CancelAll();
          m_pPositionLongOpen->ClosePosition();
          m_pPositionShortOpen->ClosePosition();
          m_stateTrade = ETradeDone;
        }
        else {
          if ( 0 < m_sma5min.Slope() ) {
            //pPosition->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Buy, 1 );
            ++m_nTransitions;
            //m_ss.str( "" );
            //m_ss << "Ordered: " << quote.DateTime() << "; ";
            m_pPositionShortOpen->ClosePosition();
            //m_pOrdersOutstandingShorts->AddOrderFilling( m_pOrder );
            m_pOrder.reset();
            m_pOrder = m_pPositionLongOpen->PlaceOrder( ou::tf::OrderType::Market, ou::tf::OrderSide::Buy, 1 );
            m_ceLongs.AddLabel( quote.DateTime(), quote.Ask(), "Long b" );
            m_stateTrade = ETradeLong;
          }
        }
      }
      break;
    case ETradeDone:
      break;
    }
  }
}

void Strategy::HandleTrade( const ou::tf::CTrade& trade ) {
  m_trades.Append( trade );
  m_barFactory.Add( trade );
}

void Strategy::HandleSimulationComplete( void ) {
  m_ss.str( "" );
  m_ss << m_nTransitions << " changes, ";
  m_pPositionLongOpen->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShortOpen->EmitStatus( m_ss );
  m_ss << ". ";
  m_sim->EmitStats( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleExecution( ou::tf::CPosition::execution_delegate_t del ) {
  m_ss << "Exec: " << del.second.GetTimeStamp() << ": ";
  m_pPositionLongOpen->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShortOpen->EmitStatus( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleCommission( const ou::tf::CPosition* pPosition ) {
  m_ss.str( "" );
  m_pPositionLongOpen->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShortOpen->EmitStatus( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleBarCompletion( const ou::tf::CBar& bar ) {
  m_ceBars.AddBar( bar );
  m_ceVolume.Add( bar.DateTime(), bar.Volume() );
}


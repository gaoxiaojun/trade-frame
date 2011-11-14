/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#include <OUCommon/TimeSource.h>

#include "SimulateOrderExecution.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

int CSimulateOrderExecution::m_nExecId( 1000 );

CSimulateOrderExecution::CSimulateOrderExecution(void)
: m_dtQueueDelay( milliseconds( 500 ) ), m_dblCommission( 1.00 ), 
  m_ea( EAQuotes ),
  m_nOrderQuanRemaining( 0 )
{
}

CSimulateOrderExecution::~CSimulateOrderExecution(void) {
}

void CSimulateOrderExecution::NewTrade( const CTrade& trade ) {
  if ( EATrades == m_ea ) {
    CQuote quote( trade.DateTime(), trade.Trade(), trade.Volume(), trade.Trade(), trade.Volume() );
    if ( !m_lDelayOrder.empty() || !m_lDelayCancel.empty() ) {
      ProcessDelayQueues( quote );
    }
  }
}

void CSimulateOrderExecution::NewQuote( const CQuote& quote ) {
  if ( EAQuotes == m_ea ) {
    if ( !m_lDelayOrder.empty() || !m_lDelayCancel.empty() ) {
      ProcessDelayQueues( quote );
    }
  }
}

void CSimulateOrderExecution::SubmitOrder( pOrder_t pOrder ) {
  m_lDelayOrder.push_back( pOrder );
}

void CSimulateOrderExecution::CancelOrder( COrder::idOrder_t nOrderId ) {
  structCancelOrder co( ou::CTimeSource::Instance().Internal(), nOrderId );
  m_lDelayCancel.push_back( co );
}

void CSimulateOrderExecution::CalculateCommission( COrder* pOrder, CTrade::tradesize_t quan ) {
  // COrder should have commission calculation?
  if ( 0 != quan ) {
    if ( NULL != OnCommission ) {
      double dblCommission( 0 );
      switch ( pOrder->GetInstrument()->GetInstrumentType() ) {
        case InstrumentType::ETF:
        case InstrumentType::Stock:
          dblCommission = 0.005 * (double) quan;
          if ( 1.00 > dblCommission ) dblCommission = 1.00;
          break;
        case InstrumentType::Option:
          dblCommission = 0.95 * (double) quan;
          break;
        case InstrumentType::Future:
          dblCommission = 2.50 * (double) quan;  // GC futures have this commission
          break;
        case InstrumentType::Currency:
          break;
      }
      OnCommission( pOrder->GetOrderId(), dblCommission );
    }
  }
}

void CSimulateOrderExecution::ProcessDelayQueues( const CQuote &quote ) {

  if ( ( 0.0 == quote.Ask() ) || ( 0.0 == quote.Bid() ) || ( 0 == quote.AskSize() ) || ( 0 == quote.BidSize() ) ) {
    return;
  }

  // process cancels list
  bool bNoMore = false;
  while ( !bNoMore && !m_lDelayCancel.empty() ) {
    structCancelOrder &co = m_lDelayCancel.front();
    if ( ( co.dtCancellation + m_dtQueueDelay ) < quote.DateTime() ) {
      bNoMore = true;  // havn't waited long enough to simulate cancel submission
    }
    else {
      structCancelOrder co = m_lDelayCancel.front();
      m_lDelayCancel.pop_front();
      bool bOrderFound = false;
      for ( lDelayOrder_iter_t iter = m_lDelayOrder.begin(); iter != m_lDelayOrder.end(); ++iter ) {
        if ( co.nOrderId == (*iter)->GetOrderId() ) {
          // perform cancellation on in-process order
          if ( 0 != m_pCurrentOrder ) {
            if ( co.nOrderId == m_pCurrentOrder->GetOrderId() ) {
              m_pCurrentOrder.reset();
            }
          }
          if ( 0 != OnOrderCancelled ) {
            CalculateCommission( m_pCurrentOrder.get(), m_nOrderQuanProcessed );
            OnOrderCancelled( co.nOrderId );
          }
          m_lDelayOrder.erase( iter );
          bOrderFound = true;
          break;
        }
      }
      if ( !bOrderFound ) {  // need an event for this, as it could be legitimate crossing execution prior to cancel
        std::cout << "no order found to cancel: " << co.nOrderId << std::endl;
        if ( NULL != OnNoOrderFound ) OnNoOrderFound( co.nOrderId );
      }
    }
  }

  // process orders list
  // only handles first in queue
  // need to build and maintain order book, particularily for handling limit orders
  if ( ( 0  == m_pCurrentOrder ) && ( !m_lDelayOrder.empty() ) ) {
    m_pCurrentOrder = m_lDelayOrder.front();
    if ( ( m_pCurrentOrder->GetDateTimeOrderSubmitted() + m_dtQueueDelay ) < quote.DateTime() ) {
      m_lDelayOrder.pop_front();
      m_nOrderQuanRemaining = m_pCurrentOrder->GetQuanOrdered();
      m_nOrderQuanProcessed = 0;
      assert( 0 != m_nOrderQuanRemaining );
    }
    else {
      m_pCurrentOrder.reset();
      m_nOrderQuanRemaining = 0;  // is this needed?
    }
  }
  if ( 0 != m_pCurrentOrder ) {
    assert( 0 != m_nOrderQuanRemaining );
    if ( ( 0 == quote.AskSize() ) || ( 0 == quote.BidSize() ) ) {
      std::runtime_error( "zero ask or zero bid size" );
      // need to requeue the order here
    }
    else {
      CTrade::tradesize_t quanAvail;
      double dblPrice;
      OrderSide::enumOrderSide orderSide = m_pCurrentOrder->GetOrderSide();
      switch ( orderSide ) {
        case OrderSide::Buy:
          quanAvail = std::min<CTrade::tradesize_t>( m_nOrderQuanRemaining, quote.AskSize() );
          dblPrice = quote.Ask();
          break;
        case OrderSide::Sell:
          quanAvail = std::min<CTrade::tradesize_t>( m_nOrderQuanRemaining, quote.BidSize() );
          dblPrice = quote.Bid();
          break;
        default:
          throw std::runtime_error( "CSimulateOrderExecution::ProcessDelayQueues unknown order side" );
          break;
      }

      switch ( m_pCurrentOrder->GetOrderType() ) {
        case OrderType::Market: 
          {
          std::string id;
          GetExecId( &id );
          CExecution exec( dblPrice, quanAvail, orderSide, "SIMMkt", id );
          if ( NULL != OnOrderFill ) 
            OnOrderFill( m_pCurrentOrder->GetOrderId(), exec );
          m_nOrderQuanRemaining -= quanAvail;
          m_nOrderQuanProcessed += quanAvail;
          }
          break;
        case OrderType::Limit: {
          // need to handle order book
          double dblLimitOrderPrice = m_pCurrentOrder->GetPrice1();
          assert( 0 < dblLimitOrderPrice );
          switch ( orderSide ) {
            case OrderSide::Buy:
              if ( quote.Ask() <= dblLimitOrderPrice ) {
                std::string id;
                GetExecId( &id );
                CExecution exec( quote.Ask(), quanAvail, orderSide, "SIMLmtBuy", id );
                if ( NULL != OnOrderFill ) 
                  OnOrderFill( m_pCurrentOrder->GetOrderId(), exec );
                m_nOrderQuanRemaining -= quanAvail;
                m_nOrderQuanProcessed += quanAvail;
              }
              break;
            case OrderSide::Sell:
              if ( quote.Bid() >= dblLimitOrderPrice ) {
                std::string id;
                GetExecId( &id );
                CExecution exec( quote.Bid(), quanAvail, orderSide, "SIMLmtSell", id );
                if ( NULL != OnOrderFill ) 
                  OnOrderFill( m_pCurrentOrder->GetOrderId(), exec );
                m_nOrderQuanRemaining -= quanAvail;
                m_nOrderQuanProcessed += quanAvail;
              }
              break;
            default:
              break;
          }
          break;
        }
      }
      if ( 0 == m_nOrderQuanRemaining ) {
        CalculateCommission( m_pCurrentOrder.get(), m_nOrderQuanProcessed );
        m_pCurrentOrder.reset();
      }
    }
  }
}

} // namespace tf
} // namespace ou

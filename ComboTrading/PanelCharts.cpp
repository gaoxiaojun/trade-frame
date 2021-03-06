/************************************************************************
 * Copyright(c) 2015, One Unified. All rights reserved.                 *
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

// started December 6, 2015, 1:26 PM

#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/phoenix/core/argument.hpp>

#include <wx/splitter.h>
#include <wx/sizer.h>
//#include <wx/dc.h>
#include <wx/icon.h>
#include <wx/menu.h>

#include <TFVuTrading/DialogPickSymbol.h>
#include <TFIQFeed/BuildSymbolName.h>

// need to check why this exists
#include <wx/toplevel.h>

#include "PanelCharts.h"
#include "TreeItemGroup.h"

// 2016/10/16 todo:
//   check serialization of instruments
//   save and retrieve data for charting
//   load data for charting
//   watch/unwatch live data on the charts
//   implement zoom/pan ability on chart

namespace ou { // One Unified
namespace tf { // TradeFrame

PanelCharts::PanelCharts( void ): wxPanel() {
  Init();
}

PanelCharts::PanelCharts( 
  wxWindow* parent, wxWindowID id, 
  const wxPoint& pos, 
  const wxSize& size, 
  long style )
{
  
  Init();
  Create(parent, id, pos, size, style);
  
}

PanelCharts::~PanelCharts() {
}

void PanelCharts::Init( void ) {
  m_pDialogPickSymbol = 0;
  m_pTreeOps = 0;
  //m_winChart = 0;
  m_pwinDetail = 0;
  m_pDialogPickSymbol = 0;
}

bool PanelCharts::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) {

  wxPanel::Create( parent, id, pos, size, style );

  CreateControls();
  if (GetSizer())     {
      GetSizer()->SetSizeHints(this);
  }
  Centre();
  return true;
}

/*
   m_sizerPM = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_sizerPM);

  //m_scrollPM = new wxScrolledWindow( m_pFPPOE, -1, wxDefaultPosition, wxSize(200, 400), wxVSCROLL );
  m_scrollPM = new wxScrolledWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL );
  m_sizerPM->Add(m_scrollPM, 1, wxGROW|wxALL, 5);
  m_scrollPM->SetScrollbars(1, 1, 0, 0);

  m_sizerScrollPM = new wxBoxSizer(wxVERTICAL);
  m_scrollPM->SetSizer( m_sizerScrollPM );
*/

/*
void TreeItemProjectorArea::ConnectToSelectionEvent( TreeItemSceneElementBase* p ) {
  namespace args = boost::phoenix::arg_names;
  p->m_signalSelectionEventSetSelected.connect( boost::phoenix::bind( &TreeItemProjectorArea::HandleSetSelected, this, args::arg1 ) );
  p->m_signalSelectionEventRemoveSelected.connect( boost::phoenix::bind( &TreeItemProjectorArea::HandleRemoveSelected, this, args::arg1 ) );
}

} */

void PanelCharts::CreateControls() {    

    PanelCharts* itemPanel1 = this;

    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(sizerMain);

  // splitter
  wxSplitterWindow* splitter;
  splitter = new wxSplitterWindow( this );
  splitter->SetMinimumPaneSize(10);
  splitter->SetSashGravity(0.2);

  // tree
  //wxTreeCtrl* tree;
//  m_pTreeSymbols = new wxTreeCtrl( splitter );
  //wxTreeItemId idRoot = m_pTreeSymbols->AddRoot( "/", -1, -1, new CustomItemData( CustomItemData::Root, m_eLatestDatumType ) );
//  wxTreeItemId idRoot = m_pTreeSymbols->AddRoot( "Collections", -1, -1, new CustomItemBase );
  //m_pHdf5Root->Bind( wxEVT_COMMAND_TREE_SEL_CHANGED, &PanelChartHdf5::HandleTreeEventItemActivated, this, m_pHdf5Root->GetId() );
//  m_pFrameMain->Bind( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, &AppLiveChart::HandleTreeEventItemActivated, this, m_pHdf5Root->GetId()  );
//  m_pFrameMain->Bind( wxEVT_COMMAND_TREE_SEL_CHANGED, &AppLiveChart::HandleTreeEventItemActivated, this, m_pHdf5Root->GetId()  );
  //m_pTreeSymbols->Bind( wxEVT_COMMAND_TREE_ITEM_MENU, &PanelCharts::HandleTreeItemMenu, this, m_pTreeSymbols->GetId()  );
  //m_pTreeSymbols->AppendItem( idRoot, "equities" );
  //m_pTreeSymbols->AppendItem( idRoot, "futures" );
  //m_pTreeSymbols->AppendItem( idRoot, "foptions" );
  //m_pTreeSymbols->AppendItem( idRoot, "portfolios" );
  
  m_pTreeOps = new ou::tf::TreeOps( splitter );
  m_pTreeOps->PopulateResources( m_baseResources );
  
  wxTreeItemId id = m_pTreeOps->AddRoot( "Root" );  // can be renamed
  boost::shared_ptr<TreeItemRoot> p( new TreeItemRoot( id, m_baseResources, m_resources ) );
  m_pTreeOps->SetRoot( p );
  //m_pTreeItemRoot.reset( new TreeItemRoot( id, m_resources ) );
  //m_mapDecoder.insert( mapDecoder_t::value_type( id.GetID(), m_pTreeItemRoot ) );

  // panel for right side of splitter
  wxPanel* panelSplitterRightPanel;
  panelSplitterRightPanel = new wxPanel( splitter );

  splitter->SplitVertically( m_pTreeOps, panelSplitterRightPanel, 0 );
  sizerMain->Add( splitter, 1, wxGROW|wxALL, 5 );

  // sizer for right side of splitter
  wxBoxSizer* sizerRight;
  sizerRight = new wxBoxSizer( wxVERTICAL );
  panelSplitterRightPanel->SetSizer( sizerRight );

  // initialize the tree
  //m_pHdf5Root->DeleteChildren( m_pHdf5Root->GetRootItem() );

  namespace args = boost::phoenix::arg_names;
  m_resources.signalNewInstrumentViaDialog.connect( boost::phoenix::bind( &PanelCharts::HandleNewInstrumentRequest, this, args::arg1 ) );
  m_resources.signalLoadInstrument.connect( boost::phoenix::bind( &PanelCharts::HandleLoadInstrument, this, args::arg1 ) );
  
  m_de.signalLookupDescription.connect( boost::phoenix::bind( &PanelCharts::HandleLookUpDescription, this, args::arg1, args::arg2 ) );
  m_de.signalComposeComposite.connect( boost::phoenix::bind( &PanelCharts::HandleComposeComposite, this, args::arg1 ) );
  
  m_pTreeOps->signalChanging.connect( boost::phoenix::bind( &PanelCharts::HandleTreeOpsChanging, this, args::arg1 ) );
  
  Bind( wxEVT_CLOSE_WINDOW, &PanelCharts::OnClose, this );  // start close of windows and controls
  
  //Bind( wxEVT_TIMER, &PanelCharts::HandleGuiRefresh, this, m_timerGuiRefresh.GetId() );
  //m_timerGuiRefresh.SetOwner( this );
  
  //m_pwinDetail = new ChartInteractive( panelSplitterRightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
  //sizerRight->Add( m_pwinDetail, 1, wxALL|wxEXPAND, 5);

}

void PanelCharts::SetProviders( pProvider_t pData1Provider, pProvider_t pData2Provider, pProvider_t pExecutionProvider ) {
  bool b( m_pData1Provider.get() != pData1Provider.get() );
  m_pData1Provider = pData1Provider;
  m_pData2Provider = pData2Provider;
  m_pExecutionProvider = pExecutionProvider;
  if ( b ) {
    for ( mapInstrumentWatch_t::iterator iter = m_mapInstrumentWatch.begin(); m_mapInstrumentWatch.end() != iter; ++iter ) {
      iter->second->SetProvider( m_pData1Provider );
    }
  }
}

void PanelCharts::HandleTreeOpsChanging( wxTreeItemId id ) {
  if ( 0 != m_pwinDetail ) {
    delete m_pwinDetail;
    m_pwinDetail = 0;
  }
}

PanelCharts::pWatch_t PanelCharts::HandleNewInstrumentRequest( const Resources::ENewInstrumentLock lock ) {
  
  assert( 0 == m_pDialogPickSymbol );
  
  m_pDialogPickSymbol = new ou::tf::DialogPickSymbol( this );
  m_pDialogPickSymbol->SetDataExchange( &m_de );
  
  switch ( lock ) {
    case Resources::ENewInstrumentLock::LockFuturesOption:
      m_pDialogPickSymbol->SetFuturesOptionOnly();
      break;
    case Resources::ENewInstrumentLock::LockOption:
      m_pDialogPickSymbol->SetOptionOnly();
      break;
    case Resources::ENewInstrumentLock::NoLock:
      break;
  }
  
  int status = m_pDialogPickSymbol->ShowModal();
  
  pWatch_t pInstrumentWatch;
  
  switch ( status ) {
    case wxID_CANCEL:
      //m_pDialogPickSymbolCreatedInstrument.reset();
      break;
    case wxID_OK:
      if ( 0 != m_pDialogPickSymbolCreatedInstrument.get() ) {
        pInstrumentWatch = LoadInstrument( m_pDialogPickSymbolCreatedInstrument );
      }
      break;
  }
  
  m_pDialogPickSymbol->Destroy();
  m_pDialogPickSymbol = 0;
  
  m_pDialogPickSymbolCreatedInstrument.reset();
  
  return pInstrumentWatch;
}

PanelCharts::pWatch_t PanelCharts::HandleLoadInstrument( const std::string& name ) {
  return LoadInstrument( signalLoadInstrument( name ) );
}

PanelCharts::pWatch_t PanelCharts::LoadInstrument( pInstrument_t pInstrument ) {
  
  pWatch_t pInstrumentWatch;
  
  const ou::tf::Instrument::idInstrument_t sInstrumentId( pInstrument->GetInstrumentName() );
  mapInstrumentWatch_t::iterator iter = m_mapInstrumentWatch.find( sInstrumentId );
  if ( m_mapInstrumentWatch.end() == iter ) {
    pInstrumentWatch.reset( new ou::tf::Watch( pInstrument, m_pData1Provider ) );
    m_mapInstrumentWatch.insert( mapInstrumentWatch_t::value_type( sInstrumentId, pInstrumentWatch ) );
    signalRegisterInstrument( pInstrument );
  }
  else {
    pInstrumentWatch = iter->second;
  }
  return pInstrumentWatch;
}

// extract this sometime because the string builder might be used elsewhere
void PanelCharts::BuildInstrument( const DialogPickSymbol::DataExchange& pde, pInstrument_t& pInstrument ) {
  std::string sKey( pde.sIQFSymbolName );
  switch ( pde.it ) {
    case InstrumentType::Stock: {
      ValuesForBuildInstrument values( sKey, pde.sCompositeName, pde.sIBSymbolName, pInstrument, 0 );
      signalBuildInstrument( values );
    }
      break;
    case InstrumentType::Option:
    case InstrumentType::FuturesOption:
    {
      boost::uint16_t month( pde.month + 1 ); // month is 0 based
      boost::uint16_t day( pde.day ); // day is 1 based
      sKey += "-" + boost::lexical_cast<std::string>( pde.year )
        + ( ( 9 < month ) ? "" : "0" ) + boost::lexical_cast<std::string>( month ) 
        + ( ( 9 < day ) ? "" : "0" ) + boost::lexical_cast<std::string>( day );
      sKey += "-";
      sKey += pde.os;
      sKey += "-" + boost::lexical_cast<std::string>( pde.dblStrike )
        ;
      ValuesForBuildInstrument values( sKey, pde.sCompositeName, pde.sIBSymbolName, pInstrument, day );
      signalBuildInstrument( values );
    }
      break;
    case InstrumentType::Future:
    {
      boost::uint16_t month( pde.month + 1 ); // month is 0 based
      boost::uint16_t day( pde.day ); // day is 1 based
      sKey += "-" + boost::lexical_cast<std::string>( pde.year )
        + ( ( 9 < month ) ? "" : "0" ) + boost::lexical_cast<std::string>( month )
        + ( ( 0 == day ) ? "" : ( ( ( 9 < day ) ? "" : "0" ) + boost::lexical_cast<std::string>( day ) ) );
        ;
      ValuesForBuildInstrument values( sKey, pde.sCompositeName, pde.sIBSymbolName, pInstrument, day );
      signalBuildInstrument( values );
    }
      break;
  }
  
}

void PanelCharts::HandleLookUpDescription( const std::string& sSymbol, std::string& sDescription ) {
  signalLookUpDescription( sSymbol, sDescription );
}

void PanelCharts::HandleComposeComposite( DialogPickSymbol::DataExchange* pde ) {
  pde->sCompositeName = "";
  pde->sCompositeDescription = "";
  switch ( pde->it ) {
    case ou::tf::InstrumentType::Stock:
      pde->sCompositeName = pde->sIQFSymbolName;
      break;
    case ou::tf::InstrumentType::Option:
      pde->sCompositeName 
        = ou::tf::iqfeed::BuildOptionName( pde->sIQFSymbolName, pde->year, pde->month + 1, pde->day, pde->dblStrike, pde->os );
      break;
    case ou::tf::InstrumentType::Future:
      pde->sCompositeName
        = ou::tf::iqfeed::BuildFuturesName( pde->sIQFSymbolName, pde->year, pde->month + 1 );
      break;
    case ou::tf::InstrumentType::FuturesOption:
      pde->sCompositeName 
        = ou::tf::iqfeed::BuildFuturesOptionName( pde->sIQFSymbolName, pde->year, pde->month + 1, pde->dblStrike, pde->os );
      break;
    default: 
      throw std::runtime_error( "PanelCharts::HandleComposeComposite: unknown instrument type" );
      break;
  }
  if ( "" != pde->sCompositeName ) {
    signalLookUpDescription( pde->sCompositeName, pde->sCompositeDescription );
    if ( "" != pde->sCompositeDescription ) { // means we have a satisfactory iqfeed symbol
      BuildInstrument( m_de, m_pDialogPickSymbolCreatedInstrument );
    }
  }
}

// IB has populated instrument with ContractID
void PanelCharts::InstrumentUpdated( pInstrument_t pInstrument ) {
  if ( 0 != m_pDialogPickSymbol ) {
    if ( pInstrument.get() == m_pDialogPickSymbolCreatedInstrument.get() ) {
      // expecting contract id to already exist in instrument
      // might put a lock on instrument changes until contract comes back
      assert( 0 != pInstrument->GetContract() );
      m_pDialogPickSymbol->UpdateContractId( pInstrument->GetContract() );
    }
    else {
      std::cout << "PanelCharts::InstrumentUpdated error:  not expected instrument" << std::endl;
    }
  }
}

void PanelCharts::OnClose( wxCloseEvent& event ) {
  // Exit Steps: #2 -> FrameMain::OnClose
//  if ( 0 != OnPanelClosing ) OnPanelClosing();
  // event.Veto();  // possible call, if needed
  // event.CanVeto(); // if not a 
  //m_timerGuiRefresh.Stop();
  event.Skip();  // auto followed by Destroy();
}

void PanelCharts::Save( boost::archive::text_oarchive& oa) {
  //auto p = dynamic_cast<TreeItemRoot*>( m_pTreeOps->GetRoot().get() );
  //oa & *p;
  m_pTreeOps->Save<TreeItemRoot>( oa );
}

void PanelCharts::Load( boost::archive::text_iarchive& ia) {
  //auto p = dynamic_cast<TreeItemRoot*>( m_pTreeOps->GetRoot().get() );
  //ia & *p;
  m_pTreeOps->Load<TreeItemRoot>( ia );
}

wxBitmap PanelCharts::GetBitmapResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon PanelCharts::GetIconResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullIcon;
}

} // namespace tf
} // namespace ou

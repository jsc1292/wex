
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/tokenzr.h>

#include "wex/extgrid.h"

wxExtGridCellAttrProvider::wxExtGridCellAttrProvider(bool highlight_r0c0, bool hide_00)
{
    m_attrForOddRows = new wxGridCellAttr;
    m_attrForOddRows->SetBackgroundColour(wxColour(240,240,240));
	m_attrRow0Col0 = NULL;
	if (highlight_r0c0)
	{
		m_attrRow0Col0 = new wxGridCellAttr;
		m_attrRow0Col0->SetBackgroundColour(wxColour("SLATE BLUE"));
		m_attrRow0Col0->SetTextColour(*wxWHITE);
		m_attrRow0Col0->SetFont( *wxNORMAL_FONT );
	}

	m_attrCell00 = NULL;
	if (hide_00)
	{
		m_attrCell00 = new wxGridCellAttr;
		m_attrCell00->SetBackgroundColour(wxColour("SLATE BLUE"));
		m_attrCell00->SetTextColour(wxColour("SLATE_BLUE"));
	}
}

wxExtGridCellAttrProvider::~wxExtGridCellAttrProvider()
{
    m_attrForOddRows->DecRef();
	if (m_attrRow0Col0)
		m_attrRow0Col0->DecRef();
	if (m_attrCell00)
		m_attrCell00->DecRef();
}

wxGridCellAttr *wxExtGridCellAttrProvider::GetAttr(int row, int col,
                           wxGridCellAttr::wxAttrKind  kind /* = wxGridCellAttr::Any */) const
{
    wxGridCellAttr *attr = wxGridCellAttrProvider::GetAttr(row, col, kind);

	if ( m_attrCell00 && row==0 && col==0 )
    {
        if ( !attr )
        {
            attr = m_attrCell00;
            attr->IncRef();
        }
        else
        {
            if ( !attr->HasBackgroundColour() )
            {
                wxGridCellAttr *attrNew = attr->Clone();
                attr->DecRef();
                attr = attrNew;
				attr->SetBackgroundColour(wxColour("SLATE BLUE"));
				attr->SetTextColour(wxColour("SLATE BLUE"));
            }
        }

		return attr;
    }


	if ( m_attrRow0Col0 && (row==0||col==0) )
    {
        if ( !attr )
        {
            attr = m_attrRow0Col0;
            attr->IncRef();
        }
        else
        {
            if ( !attr->HasBackgroundColour() )
            {
                wxGridCellAttr *attrNew = attr->Clone();
                attr->DecRef();
                attr = attrNew;
				attr->SetBackgroundColour(wxColour("SLATE BLUE"));
				attr->SetTextColour(*wxWHITE);
				attr->SetFont( *wxNORMAL_FONT );
            }
        }

		return attr;
    }

    if ( row % 2 )
    {
        if ( !attr )
        {
            attr = m_attrForOddRows;
            attr->IncRef();
        }
        else
        {
            if ( !attr->HasBackgroundColour() )
            {
                wxGridCellAttr *attrNew = attr->Clone();
                attr->DecRef();
                attr = attrNew;
                attr->SetBackgroundColour(*wxLIGHT_GREY);
            }
        }
    }

    return attr;
}


BEGIN_EVENT_TABLE(wxExtGridCtrl, wxGrid)
	EVT_GRID_CELL_CHANGE( wxExtGridCtrl::OnGridCellChange)
	EVT_GRID_SELECT_CELL( wxExtGridCtrl::OnGridCellSelect)
	EVT_GRID_RANGE_SELECT( wxExtGridCtrl::OnGridRangeSelect)
	EVT_GRID_EDITOR_HIDDEN( wxExtGridCtrl::OnGridEditorHidden)
	EVT_GRID_EDITOR_SHOWN( wxExtGridCtrl::OnGridEditorShown)
	EVT_GRID_LABEL_LEFT_CLICK( wxExtGridCtrl::OnGridLabelClick )
	EVT_KEY_DOWN( wxExtGridCtrl::OnGridKey )
END_EVENT_TABLE()

wxExtGridCtrl::wxExtGridCtrl(wxWindow *parent, int id, 
	const wxPoint &pos, const wxSize &sz)
	: wxGrid(parent, id, pos, sz)
{
	bSendPasteEvent = true;
	bCopyPaste = true;
	bSkipSelect = false;

	mSelTopRow = mSelBottomRow = -1;
	mSelLeftCol = mSelRightCol = -1;

	mLastSelTopRow = -1;
	mLastSelBottomRow = -1;
	mLastSelLeftCol = -1;
	mLastSelRightCol = -1;
}

void wxExtGridCtrl::EnablePasteEvent(bool b)
{
	bSendPasteEvent = b;
}

void wxExtGridCtrl::GetSelRange(int *top, int *bottom, int *left, int *right)
{
	if (top) *top = mSelTopRow;
	if (bottom) *bottom = mSelBottomRow;
	if (left) *left = mSelLeftCol;
	if (right) *right = mSelRightCol;
}

void wxExtGridCtrl::GetLastSelRange(int *top, int *bottom, int *left, int *right)
{
	if (top) *top = mLastSelTopRow;
	if (bottom) *bottom = mLastSelBottomRow;
	if (left) *left = mLastSelLeftCol;
	if (right) *right = mLastSelRightCol;
}

void wxExtGridCtrl::OnGridKey(wxKeyEvent &evt)
{
	if (bCopyPaste && evt.CmdDown())
	{
		int key = tolower(evt.GetKeyCode());
		if (key=='c')
			Copy();
		else if (key=='v')
			Paste();
		else
			evt.Skip();
	}
	else
		evt.Skip();
}

void wxExtGridCtrl::OnGridLabelClick(wxGridEvent &evt)
{
	mSelLeftCol = evt.GetCol();
	mSelRightCol = evt.GetCol();
	mSelTopRow = 0;
	mSelBottomRow = GetNumberRows()-1;

	
	if (bCopyPaste) 
	{
		SetFocus();
		SetGridCursor(0,evt.GetCol());
	}

	evt.Skip();
}

void wxExtGridCtrl::OnGridCellChange(wxGridEvent &evt)
{	
	mLastSelTopRow = mSelTopRow;
	mLastSelBottomRow = mSelBottomRow;
	mLastSelLeftCol = mSelLeftCol;
	mLastSelRightCol = mSelRightCol;

	mSelTopRow = -1;
	mSelBottomRow = -1;
	mSelLeftCol = -1;
	mSelRightCol = -1;

	if (bCopyPaste && evt.GetRow() >= 0 && evt.GetCol() >= 0)
	{
		wxString cell = GetCellValue(evt.GetRow(),evt.GetCol());
		cell.Replace("\n", "");
		cell.Replace("\t", "");
		SetCellValue(evt.GetRow(), evt.GetCol(), cell );
	}

	evt.Skip();
}
void wxExtGridCtrl::ResetLastSelRange()
{
	mLastSelTopRow = -1;
	mLastSelBottomRow = -1;
	mLastSelLeftCol = -1;
	mLastSelRightCol = -1;
}

void wxExtGridCtrl::OnGridCellSelect(wxGridEvent &evt)
{
	if (evt.CmdDown())
	{
		evt.Veto();
		return;
	}

	if (!bSkipSelect)
	{
		mSelTopRow = evt.GetRow();
		mSelBottomRow = mSelTopRow;
		mSelLeftCol = evt.GetCol();
		mSelRightCol = mSelLeftCol;
	}

	evt.Skip();
}

void wxExtGridCtrl::OnGridEditorHidden(wxGridEvent &evt)
{
	bSkipSelect = false;
	evt.Skip();
}

void wxExtGridCtrl::OnGridEditorShown(wxGridEvent &evt)
{
	bSkipSelect = true;
	evt.Skip();
}

void wxExtGridCtrl::OnGridRangeSelect(wxGridRangeSelectEvent &evt)
{

	if (evt.CmdDown())
	{
		evt.Veto();
		return;
	}

	if (evt.Selecting() && !bSkipSelect)
	{

		mSelTopRow = evt.GetTopRow();
		mSelBottomRow = evt.GetBottomRow();
		mSelLeftCol = evt.GetLeftCol();
		mSelRightCol = evt.GetRightCol();
	}

	evt.Skip();
}

void wxExtGridCtrl::EnableCopyPaste(bool b)
{
	bCopyPaste = b;
}

bool wxExtGridCtrl::IsCopyPasteEnabled()
{
	return bCopyPaste;
}

void wxExtGridCtrl::Copy(bool all, bool with_headers)
{
	int minrow=mSelTopRow, maxrow=mSelBottomRow;
	int mincol=mSelLeftCol, maxcol=mSelRightCol;

	if (all || mSelTopRow < 0 || mSelLeftCol < 0)
	{
		minrow = 0;
		maxrow = GetNumberRows()-1;
		mincol = 0;
		maxcol = GetNumberCols()-1;
	}

	wxString data;

	if (with_headers)
	{
		for (int c=mincol;c<=maxcol;c++)
			data += this->GetColLabelValue( c )  + wxString(c==maxcol?'\n':'\t');
	}

	for (int r = minrow;r<=maxrow;r++)
	{
		for (int c= mincol;c<=maxcol;c++)
		{
			wxString cell = GetCellValue(r,c);
			cell.Replace("\n", "");
			cell.Replace("\t", "");
			data += cell + wxString(c==maxcol?'\n':'\t');
		}
	}

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(data) );
		wxTheClipboard->Close();
	}
}

void wxExtGridCtrl::Paste(bool all)
{
	if (wxTheClipboard->Open())
	{
		wxString data;
		wxTextDataObject textobj;
		if (wxTheClipboard->GetData( textobj ))
		{
			data = textobj.GetText();
			wxTheClipboard->Close();
		}
		if (data.IsEmpty()) return;

		int currow = GetCursorRow();
		int curcol = GetCursorColumn();

		if (all) currow = curcol = 0;
#ifdef __WXMAC__
		wxArrayString lines = wxStringTokenize(data, "\r", ::wxTOKEN_RET_EMPTY_ALL);
#else
		wxArrayString lines = wxStringTokenize(data, "\n", ::wxTOKEN_RET_EMPTY_ALL);
#endif
		for (int i=0;i<(int)lines.Count();i++)
		{
			wxArrayString parts = wxStringTokenize(lines[i], "\t", ::wxTOKEN_RET_EMPTY_ALL);
			for (int p=0;p<(int)parts.Count();p++)
			{
				int r = currow + i;
				int c = curcol + p;
				if (r < GetNumberRows() && c < GetNumberCols() && r >=0 && c >= 0 )
				{
					if (!IsReadOnly( r, c ))
					{
						parts[p].Replace("\r", "");
						parts[p].Replace("\n", "");
						SetCellValue( r, c, parts[p] );
					}
				}
			}
		}

		if (bSendPasteEvent)
		{
			wxGridEvent evt(this->GetId(), ::wxEVT_GRID_CELL_CHANGE, this, -1, -1);
			this->ProcessEvent(evt);
		}
	}
}

void wxExtGridCtrl::ResizeGrid(int nrows, int ncols)
{
	Freeze();

	if (GetNumberRows() > nrows)
		DeleteRows( nrows, GetNumberRows() - nrows );

	if (GetNumberRows() < nrows)
		AppendRows( nrows - GetNumberRows() );

	if (GetNumberCols() > ncols)
		DeleteCols( ncols, GetNumberCols() - ncols );

	if (GetNumberCols() < ncols)
		AppendCols( ncols - GetNumberCols() );

	Thaw();
}
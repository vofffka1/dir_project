#pragma once
#include "wx/wx.h"
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>


typedef std::unordered_map<wxString,std::vector<wxString>> Dirs;
typedef std::tuple<wxString,wxString,std::size_t> Triple;
typedef std::unordered_map<wxString,std::size_t> Map;
typedef std::unordered_map<wxString,Map> Groups2;
typedef std::unordered_map<wxString,std::vector<Triple>> Groups;

class MyFrame : public wxFrame 
	{
	public:

		MyFrame(const wxString& title);
		~MyFrame();

	protected:

		wxDirPickerCtrl* m_dirPicker1;
		wxButton* m_Start;
		wxButton* m_ShowEmpty;
		wxButton* m_Save;
		wxButton* m_Info;
		wxChoice* m_Mode;
		wxListBox* m_GroupDirs;
		wxTextCtrl* m_DiffDirs;
		wxSpinCtrl* m_LimitValue;

		Dirs m_dirs;
		std::unordered_set<wxString> m_emptyDirs;
		Groups m_Group;


	private:
		void Binds();
		void Fill(const wxString &path);
		void Clear();
		std::pair<bool,std::size_t> MyFrame::VolumeCompare(
			const std::vector<wxString> &a,
			const std::vector<wxString> &b);
		std::pair<bool,std::size_t> MyFrame::CountCompare(
			const std::vector<wxString> &a,
			const std::vector<wxString> &b);

	};

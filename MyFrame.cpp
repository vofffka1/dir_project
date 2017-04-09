#include "MyFrame.h"
#include <wx/filedlg.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/grid.h>
#include <memory>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <vector>

class wxSimpleDirTraverser : public wxDirTraverser
	{
	public:
		wxSimpleDirTraverser(wxArrayString& files) : m_files(files) { }

		virtual wxDirTraverseResult OnFile(const wxString& WXUNUSED(filename))
			{
			return wxDIR_CONTINUE;
			}

		virtual wxDirTraverseResult OnDir(const wxString& dirname)
			{
			m_files.Add(dirname);
			return wxDIR_CONTINUE;
			}

	private:
		wxArrayString& m_files;
	};

class GridDialog : public wxDialog
	{
	public:
		GridDialog(
			const std::vector<Triple> &data)
			: wxDialog(nullptr, -1, "")
			{
			this->SetSize(600,400);
			this->SetPosition(wxPoint(100,100));
			wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
			wxGrid* lc = new wxGrid(this, wxID_ANY,wxDefaultPosition,wxSize(480,280));
			lc->EnableEditing(false);
			lc->CreateGrid(data.size(),3);
			lc->AutoSizeColumn(0,true);
			lc->SetColLabelValue(0,"First");
			lc->SetColSize(0,210);
			lc->SetColLabelValue(1,"Second");
			lc->SetColSize(1,210);
			lc->SetColLabelValue(2,"Similarity");
			lc->SetColSize(2,-1);
			std::size_t i =0;
			for (const auto &el : data)
				{
				lc->SetCellValue(i,0,std::get<0>(el));
				lc->SetCellValue(i,1,std::get<1>(el));
				lc->SetCellValue(i++,2,wxString::Format("%d",std::get<2>(el)));
				}

			topSizer->Add(lc, 1,wxEXPAND | wxALL, 5 );

			this->SetSizer(topSizer);
			this->FitInside(); 
			this->Layout();
			ShowModal();
			Destroy();
			}
	};

class EmptyDialog : public wxDialog
	{
	public:
		EmptyDialog(
			const std::unordered_set<wxString> &data)
			: wxDialog(nullptr, -1, "")
			{
			this->SetSize(400,200);
			this->SetPosition(wxPoint(100,100));
			wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
			wxTextCtrl* lc = new wxTextCtrl( this, wxID_ANY, "",wxDefaultPosition, wxDefaultSize,wxTE_MULTILINE|wxHSCROLL);
			
			for(const auto &el : data)
				lc->AppendText(el+"\n");

			topSizer->Add(lc, 1,wxEXPAND | wxALL, 5 );

			this->SetSizer(topSizer);
			this->FitInside(); 
			this->Layout();
			ShowModal();
			Destroy();
			}
	};

bool is_identity(wxFileInputStream &a,wxFileInputStream &b);
bool is_contain(const Map &a,const Map &b);

MyFrame::MyFrame(const wxString& title) : 
	wxFrame(nullptr, wxID_ANY, title,wxDefaultPosition,wxSize(550,290),
	wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX))
	{
	wxBoxSizer* m_mainSizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* m_inSizer = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer* bSizer4 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* bSizer8 = new wxBoxSizer( wxHORIZONTAL );

	auto m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Путь к исходной директории"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer8->Add( m_staticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_dirPicker1 = new wxDirPickerCtrl( this, wxID_FIRST, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE );
	bSizer8->Add( m_dirPicker1, 1, wxALL|wxEXPAND, 5 );

	bSizer4->Add( bSizer8, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer91 = new wxBoxSizer( wxHORIZONTAL );

	auto m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("Установите пороговое значение"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer91->Add( m_staticText11, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_LimitValue = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 50, 100, 90 );
	bSizer91->Add( m_LimitValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxString m_ModeChoices[] = { wxT("по количеству"),wxT("по объему") };
	int m_ModeNChoices = sizeof( m_ModeChoices ) / sizeof( wxString );
	m_Mode = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ModeNChoices, m_ModeChoices, 0 );
	m_Mode->SetSelection( 0 );
	bSizer91->Add( m_Mode, 1, wxALL, 5 );

	bSizer4->Add( bSizer91, 1, wxEXPAND, 5 );

	wxBoxSizer* m_prefBox = new wxBoxSizer( wxHORIZONTAL );

	m_Start = new wxButton( this, wxID_ANY, wxT("Начать"), wxDefaultPosition, wxDefaultSize, 0 );
	m_prefBox->Add( m_Start, 1, wxALL, 5 );

	m_ShowEmpty = new wxButton( this, wxID_ANY, wxT("Отобразить пустые"), wxDefaultPosition, wxDefaultSize, 0 );
	m_prefBox->Add( m_ShowEmpty, 1, wxALL, 5 );

	m_Save = new wxButton( this, wxID_ANY, wxT("Сохранить"), wxDefaultPosition, wxDefaultSize, 0 );
	m_prefBox->Add( m_Save, 1, wxALL, 5 );


	bSizer4->Add( m_prefBox, 1, wxEXPAND, 5 );
	m_inSizer->Add( bSizer4, 1, wxEXPAND, 5 );
	m_mainSizer->Add( m_inSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* m_outSizer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer9 = new wxBoxSizer( wxVERTICAL );

	auto m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Совпадающие директории"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer9->Add( m_staticText4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_GroupDirs = new wxListBox(this, wxID_PRINT,wxDefaultPosition,wxDefaultSize, wxArrayString(), wxLB_SINGLE);
	bSizer9->Add( m_GroupDirs, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	m_Info = new wxButton( this, wxID_INFO, wxT("Отобразить"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_Info, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	m_outSizer->Add( bSizer9, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer10 = new wxBoxSizer( wxVERTICAL );

	auto m_staticText5 = new wxStaticText( this, wxID_ANY,wxT("Уникальные директории"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer10->Add( m_staticText5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DiffDirs = new wxTextCtrl( this, wxID_ANY, "",wxDefaultPosition, wxDefaultSize,wxTE_MULTILINE|wxHSCROLL);
	bSizer10->Add( m_DiffDirs, 1, wxALL|wxEXPAND, 5 );

	m_outSizer->Add( bSizer10, 1, wxEXPAND, 5 );
	m_mainSizer->Add( m_outSizer, 1, wxEXPAND, 5 );
	this->SetSizer( m_mainSizer );
	this->Layout();
	this->Centre( wxBOTH );
	//начиная с wxWidgets 2.9.0 
	//если компилятор поддерживает C++11
	//очень удобно вместо макросов
	//использовать лямбда-выражения в качестве обработчиков
	Binds();
	
	}

MyFrame::~MyFrame(void)
	{
	}

void MyFrame::Binds()
	{
	m_dirPicker1->Bind(wxEVT_DIRPICKER_CHANGED,[&](wxFileDirPickerEvent &evt)
		{
		//если изменилась директория очищаем
		//устанавливаем стандартное пороговое значение
		Clear();
		m_LimitValue->SetValue(90);
		},wxID_FIRST);

	m_Start->Bind(wxEVT_BUTTON, [&](wxCommandEvent&)
		{
		//основной обработчкик
		Clear();
		auto root = m_dirPicker1->GetPath();
		//получаем карту директорий-файлов
		Fill(root);
		//удаляем корень
		m_dirs.erase(root);
        Groups2 data;
		std::unordered_set<wxString> dub;
		//перебор всех директорий
		for(auto it = m_dirs.cbegin();it!=m_dirs.cend();it++)
			{
			//если директория пустая - продолжаем
			if(it->second.empty())
				{
				m_emptyDirs.insert(it->first);
				continue;
				}
			//сравниваем со всеми последующими
			for(auto j = std::next(it);j!=m_dirs.cend();j++)
				{
				bool is_eq;
				std::size_t percent;
				//сравниваем в зависимости от режима
				if(m_Mode->GetSelection()==0) 
					std::tie(is_eq,percent)  = CountCompare(it->second,j->second);
				else
					std::tie(is_eq,percent) = VolumeCompare(it->second,j->second);
				if(is_eq)
					{
					//если директории эквивалентны
					//то добавляем пути в набор дубликатов
					//и сохраняем процент соответствия
					dub.insert(it->first);
					dub.insert(j->first);
					data[it->first][j->first] = percent;
					}
				}
			}
		//выводим уникальные
		for(const auto &el : m_dirs)
			if(dub.find(el.first)==dub.end() 
				&& m_emptyDirs.find(el.first)==m_emptyDirs.end())
			    m_DiffDirs->AppendText(el.first+"\n");

		//компоновка
		std::unordered_map<wxString,std::vector<Triple>> group;
		for(auto it = data.cbegin();it!=data.cend();it++)
			{
			auto tempstr = it->first;
			auto key = tempstr.substr(tempstr.find_last_of('\\')+1);

			for(const auto &el : it->second)
			    group[key].push_back(std::make_tuple(it->first,el.first,el.second));

			for(auto j = std::next(it);j!=data.cend();)
				{
				if(it->second.find(j->first)!=it->second.end()
					&& is_contain(it->second,j->second))
					{
					for(const auto &el : j->second)
			            group[key].push_back(std::make_tuple(j->first,el.first,el.second));

					j = data.erase(j);
					}
				else
					j++;
				}
			}
		//создаем список групп
		if(!group.empty())
		{
		wxArrayString strings;
		for(const auto &el : group)
			strings.Add(el.first);
			
		m_GroupDirs->Insert(strings,0);
		m_GroupDirs->SetSelection(0);
		m_Group = group;
		}
		});

	m_Save->Bind(wxEVT_BUTTON, [&](wxCommandEvent&)
		{
		//сохраняем в файл
		wxFileOutputStream outStream("log.txt");
		wxTextOutputStream out(outStream);
		for(const auto &el : m_dirs)
			{
			out<<"Directory "<<el.first<<" contain files :\n";
			for(const auto &x : el.second)
				out<<x<<'\n';
			out<<'\n';
			}
		});

	m_ShowEmpty->Bind(wxEVT_BUTTON, [&](wxCommandEvent&)
		{
		    //отображаем пустые
			EmptyDialog(m_emptyDirs).Show();
		});

	m_Info->Bind(wxEVT_BUTTON, [&](wxCommandEvent&)
		{
		//отображаем таблицу   
		GridDialog(
			m_Group[m_GroupDirs->GetStringSelection()]).Show();

		});
	}

void MyFrame::Clear()
	{
	m_DiffDirs->Clear();
	m_dirs.clear();
	m_GroupDirs->Clear();
	m_GroupDirs->Clear();
	m_Group.clear();
	m_emptyDirs.clear();
	}
//заполнение карты , ключ - директория, значение - вектор файлов
void MyFrame::Fill(const wxString &path)
	{
	wxDir dir(path);
	if ( !dir.IsOpened() )
		return;
	
	m_dirs[path] = std::vector<wxString>();
	wxArrayString files;

	dir.GetAllFiles(path, &files, wxEmptyString, wxDIR_FILES);
	for(const auto &el : files)
		m_dirs[path].push_back(el);

	wxArrayString dirList;
	wxSimpleDirTraverser traverser(dirList);
	dir.Traverse(traverser);

	for(const auto &el : dirList)
		Fill(el);
		
	}
//количественное сравнение
std::pair<bool,std::size_t> MyFrame::CountCompare(
	const std::vector<wxString> &a,
	const std::vector<wxString> &b)
	{
	std::size_t diff=0,eq = 0,
		maxsize = std::max(a.size(),b.size());
	double Limit = m_LimitValue->GetValue();
	//определяем максимально допустимое количество различных файлов
	std::size_t MaxDiff = maxsize-std::ceil(maxsize*Limit/100);
	//если количество файлов в папках отличается на большее - возврат
	if(maxsize-std::min(a.size(),b.size())>MaxDiff)
		return std::make_pair(false,0);
	//нужно для особого учета дублей
	std::unordered_set<wxString> set;
	for(const auto &f1 : a)
		{
		bool is_exist = false;
		for(const auto &f2 : b)
			{
			wxFileInputStream file(f1);
			wxFileInputStream temp(f2);
			//если размер совпадает
			//и ранее этот файл не был найден
			//и он идентичен проверяемому
			//добавляем в помеченные и отмечаем соответствие
			if(file.GetSize()==temp.GetSize() &&
				set.find(f2)== set.end() &&
				is_identity(file,temp)) 
				{
				is_exist = true;
				set.insert(f2);
				}
			if(is_exist)
				break;
			}

		if(is_exist)
			eq++;
		else if(diff > MaxDiff)
			return std::make_pair(false,0);
		else
			diff++;

		}
	return std::make_pair(true,double(100)*eq/maxsize);
	}
//объемное сравнение
//принцип такой-же
std::pair<bool,std::size_t> MyFrame::VolumeCompare(
	const std::vector<wxString> &a,
	const std::vector<wxString> &b)
	{
	std::size_t diff=0,eq = 0;
	std::size_t V1=0,V2=0;
	for(const auto &f1 : a)
		{
		wxFile file(f1);
		auto temp = file.Length();
		if(temp != wxInvalidOffset)
			V1+=temp;
		}

	for(const auto &f2 : b)
		{
		wxFile file(f2);
		auto temp = file.Length();
		if(temp != wxInvalidOffset)
			V2+=temp;
		}
	std::size_t maxvolume = std::max(V1,V2);
	double Limit = m_LimitValue->GetValue();
	std::size_t MaxDiff = maxvolume-std::ceil(maxvolume*Limit/100);
	if(maxvolume-std::min(V1,V2)>MaxDiff)
		return std::make_pair(false,0);
	std::unordered_set<wxString> set;
	for(const auto &f1 : a)
		{
		bool is_exist = false;
		wxFileInputStream file(f1);
		auto fVolume = file.GetSize();
		file.SeekI(0);
		
		for(const auto &f2 : b)
			{
			wxFileInputStream temp(f2);
			if(fVolume==temp.GetSize() &&
				set.find(f2)==set.end() &&
				is_identity(file,temp))
			{
				is_exist = true;
				set.insert(f2);
			}

			if(is_exist)
				break;
			}

		if(is_exist)
			eq+=fVolume;
		else if(diff > MaxDiff)
			return std::make_pair(false,0);		
		else
			diff+=fVolume;
		}
	return std::make_pair(true,double(100)*eq/maxvolume);
	}

bool is_identity(wxFileInputStream &a,wxFileInputStream &b)
	{
	static const std::size_t BufSize = 1024;
	static unsigned char buf1[BufSize];
	static unsigned char buf2[BufSize];
    size_t bytesLeft = a.GetSize();
    while (bytesLeft > 0)
    {
    size_t bytesToRead = wxMin((size_t) sizeof(buf1), bytesLeft);
    a.Read((void*) buf1, bytesToRead);
    b.Read((void*) buf2, bytesToRead);
	for(auto i = 0;i<BufSize;i++)
		if(buf1[i]!=buf2[i])
			return false;
    bytesLeft -= bytesToRead;
    }
	return true;
	}

bool is_contain(const Map &a,const Map &b)
	{
	return std::all_of(b.cbegin(),b.cend(),
		[&a](const std::pair<wxString,std::size_t> &p)
		{return a.find(p.first)!=a.end();});
	}


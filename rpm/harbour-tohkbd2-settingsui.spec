#
# tohkbd2-settingsui spec
# (C) kimmoli 2014
#

Name:       harbour-tohkbd2-settingsui

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    The OtherHalf Keyboard v2 settings user interface
Version:    0.0.devel
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        https://github.com/kimmoli/tohkbd2-settingsui
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(sailfishapp) >= 0.0.10
BuildRequires:  desktop-file-utils
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  qt5-qttools-linguist

Requires:   sailfishsilica-qt5 >= 0.10.9

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/i18n
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/86x86/apps/%{name}.png
/usr/bin
/usr/share/%{name}
/usr/share/applications
/usr/share/icons/hicolor/86x86/apps


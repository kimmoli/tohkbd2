Name:       harbour-tohkbd2-ambience

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    TOHKBD2 Ambience
Version:    0.1.0
Release:    1
Group:      System/GUI/Other
License:    TBD
Source0:    %{name}-%{version}.tar.bz2
BuildArch:  noarch
BuildRequires:  qt5-qttools
BuildRequires:  qt5-qmake

Requires:   ambienced
Requires:   harbour-tohkbd2

%description
Ambience for TOHKBD2 OtherHalf Keyboard

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
# Without the root directory specified it will not be removed on uninstall
%{_datadir}/ambience/%{name}
%{_datadir}/ambience/%{name}/harbour-tohkbd2.ambience
%{_datadir}/ambience/%{name}/images/*

%post

%_ambience_post

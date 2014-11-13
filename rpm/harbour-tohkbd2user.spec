#
# harbour-tohkbd2user spec
# (C) kimmoli 2014
#

Name:       harbour-tohkbd2user

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    tohkbd 2 user daemon
Version:    0.0.devel
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        https://github.com/kimmoli/tohkbd2-user
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(mlite5)

%description
userspace daemon for tohkbd2

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

%preun
dbus-send --type=method_call --dest=com.kimmoli.tohkbd2user / com.kimmoli.tohkbd2user.quit

%post
dbus-send --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig

%postun
dbus-send --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/%{name}
%{_datadir}/dbus-1/


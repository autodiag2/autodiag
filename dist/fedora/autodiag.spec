Name:           %{app_name}
Version:        %{app_version}
Release:        1%{?dist}
Summary:        Autodiag diagnostic tools

License:        GPL-3.0-or-later
URL:            https://github.com/autodiag2
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  gtk3-devel

Requires:       gtk3

%description
Autodiag is a diagnostic toolkit for vehicle communication, simulation, and analysis.

%prep
%autosetup -n %{name}-%{version}

%build
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%files
%license LICENSE*
%doc README*
/usr/bin/%{name}

%changelog
* Thu Mar 19 2026 autodiag2 <autodiag@netcourrier.com> - %{version}-1
- Initial Fedora package
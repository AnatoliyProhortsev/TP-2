#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <iterator>
#include <vector>
#include <iomanip>
#include <complex>
#include <fstream>
#include <algorithm>
#include <functional>

namespace prohorcev
{
	// формат ввода:
	// (:key1 10ull:key2 ’c’:key3 "Data":)
	// (:key2 ’c’:key1 10ull:key3 "Data":)
	// (:key3 "Data":key2 ’c’:key1 10ull:)

	struct DataStruct
	{
		unsigned long long key1;
		std::complex<double> key2;
		std::string key3;
	};

	struct DelimiterIO {
		char exp;
	};

	struct UnsignedLongLongIO {
		unsigned long long& ref;
	};

	struct ComplexIO {
		std::complex<double>& ref;
	};

	struct StringIO {
		std::string& ref;
	};

	struct LabelIO {
		std::string exp;
	};

	// scope guard для возврата состояния потока в первоначальное состояние
	class iofmtguard {
	public:
		iofmtguard(std::basic_ios< char >& s);
		~iofmtguard();
	private:
		std::basic_ios< char >& s_;
		char fill_;
		std::streamsize precision_;
		std::basic_ios< char >::fmtflags fmt_;
	};

	std::istream& operator>>(std::istream& in, DelimiterIO&& dest);
	std::istream& operator>>(std::istream& in, UnsignedLongLongIO&& dest);
	std::istream& operator>>(std::istream& in, ComplexIO&& dest);
	std::istream& operator>>(std::istream& in, StringIO&& dest);
	std::istream& operator>>(std::istream& in, LabelIO&& dest);
	std::istream& operator>>(std::istream& in, DataStruct& dest);
	std::ostream& operator<<(std::ostream& out, const DataStruct& dest);

	bool double_lt(double a, double b, double epsilon) {
		return a < b - epsilon;
	}

	bool comp(DataStruct first, DataStruct second)
	{
		if (first.key1 < second.key1)
			return true;
		else if (first.key1 > second.key1)
			return false;
		else
		{
			if (double_lt(second.key2.real(), first.key2.real(), 1e-100))
				return true;
			else if (double_lt(first.key2.real(), second.key2.real(), 1e-100))
				return false;
			else
			{
				if (first.key3.length() < second.key3.length())
					return true;
				else
					return false;
			}
		}
	}
}

int main()
{
	using prohorcev::DataStruct;

	std::vector< DataStruct > data;

	std::fstream file("input.txt");

	while (!file.eof())
	{
		std::copy(
			std::istream_iterator< DataStruct >(file),
			std::istream_iterator< DataStruct >(),
			std::back_inserter(data)
		);

		if (file.fail() && !file.eof())
			file.clear();
	}
	
	std::sort(data.begin(), data.end(), prohorcev::comp);

	std::cout << "Data:\n";

	std::copy(
		std::begin(data),
		std::end(data),
		std::ostream_iterator< DataStruct >(std::cout, "\n")
	);

	return 0;
}

namespace prohorcev
{
	std::istream& operator>>(std::istream& in, DelimiterIO&& dest)
	{
		// все перегрузки операторов ввода/вывода должны начинаться с проверки экземпляра класса sentry
		std::istream::sentry sentry(in);
		if (!sentry)
		{
			return in;
		}
		char c = '0';
		in >> c;
		if (in && (c != dest.exp))
		{
			in.setstate(std::ios::failbit);
		}
		return in;
	}

	std::istream& operator>>(std::istream& in, UnsignedLongLongIO&& dest)
	{
		std::istream::sentry sentry(in);
		if (!sentry)
			return in;

		in >> DelimiterIO{ '0' } >> dest.ref;

		return in;
	}

	std::istream& operator>>(std::istream& in, ComplexIO&& dest)
	{
		std::istream::sentry sentry(in);
		if (!sentry)
			return in;

		in >> DelimiterIO{ '#' } >> DelimiterIO{ 'c' } >> DelimiterIO{ '(' };

		double real;
		double img;

		in >> real >> img >> DelimiterIO{ ')' };
		dest.ref = std::complex<double>(real, img);

		return in;
	}

	std::istream& operator>>(std::istream& in, StringIO&& dest)
	{
		std::istream::sentry sentry(in);
		if (!sentry)
		{
			return in;
		}
		return std::getline(in >> DelimiterIO{ '"' }, dest.ref, '"');
	}

	std::istream& operator>>(std::istream& in, LabelIO&& dest)
	{
		std::istream::sentry sentry(in);
		if (!sentry)
		{
			return in;
		}
		std::string data = "";
		if ((in >> StringIO{ data }) && (data != dest.exp))
		{
			in.setstate(std::ios::failbit);
		}
		return in;
	}

	std::istream& operator>>(std::istream& in, DataStruct& dest)
	{
		std::istream::sentry sentry(in);
		if (!sentry)
		{
			return in;
		}
		DataStruct input;

		{
			using sep = DelimiterIO;
			using label = LabelIO;
			using ull = UnsignedLongLongIO;
			using str = StringIO;
			using cmplx = ComplexIO;

			bool keys[]{ false, false, false };
			char ch;

			in >> sep{ '(' };
			in >> sep{ ':' };


			for (int i = 0; i < 3; i++)
			{
				in >> label{ "key" };
				in >> ch;
				switch (ch)
				{
				case '1':
					if (keys[0])
					{
						in.setstate(std::ios::failbit);
						return in;
					}
					in >> ull{ input.key1 };
					keys[0] = true;
					break;
				case '2':
					if (keys[1])
					{
						in.setstate(std::ios::failbit);
						return in;
					}
					in >> cmplx{ input.key2 };
					keys[1] = true;
					break;
				case '3':
					if (keys[2])
					{
						in.setstate(std::ios::failbit);
						return in;
					}
					in >> str{ input.key3 };
					keys[2] = '0';
					break;
				default:
					in.setstate(std::ios::failbit);
					return in;
				}

				in >> sep{ ':' };
			}

			in >> sep{ ')' };
		}

		if (in)
		{
			dest = input;
		}
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const DataStruct& src)
	{
		std::ostream::sentry sentry(out);
		if (!sentry)
		{
			return out;
		}
		iofmtguard fmtguard(out);
		out << "{ ";
		out << "key1: 0" << src.key1;
		out << ": key2: " << src.key2;
		out << ": key3: " << src.key3;
		out << " }";
		return out;
	}

	iofmtguard::iofmtguard(std::basic_ios< char >& s) :
		s_(s),
		fill_(s.fill()),
		precision_(s.precision()),
		fmt_(s.flags())
	{}

	iofmtguard::~iofmtguard()
	{
		s_.fill(fill_);
		s_.precision(precision_);
		s_.flags(fmt_);
	}
}

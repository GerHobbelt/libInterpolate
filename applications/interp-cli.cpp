#include<iostream>
#include<string>
#include<algorithm>
#include<list>
#include<stdexcept>

#include<boost/program_options.hpp>

#include <libInterpolate/libInterpolate_version.h>
#include <libInterpolate/Interpolate.hpp>
#include <libInterpolate/AnyInterpolator.hpp>
#include <libInterpolate/Utils/ReadFunction.hpp>

using namespace std;
namespace po = boost::program_options;


void print_version()
{
  cout<<"interp-cli - linked against libInterp version "<< libInterpolate_VERSION_FULL << endl;
}

void print_usage(char prog_name[])
{

  cout<<"usage: "<<prog_name<<" [OPTIONS] interpolation-data-file x-values-file output-file"<<"\n";
  
}

void print_documentation( )
{
  cout<<"Reads x-y pairs from a file and interpolates to x values listed in another file.\n"
      <<"\n"
      <<"The interpoalted data (data that is interpolated from) is contained in a gnuplot-style text file,\n"
      <<"with each x-y pair on a new line, separated by white space.\n"
      <<"The x points to interpoalte to are contained in plain text file, with each value on a new line.\n"
      <<"\n"
      <<"Notes:\n"
      <<"\tthe x values to be interpolated to can also be stored in a gnuplot-style text file. If the file\n"
      <<"\tcontaines more than one column, only the first will be used.\n"
      <<"\n";
}

_1D::AnyInterpolator<double> create(std::string type)
{
  if( type == "linear" )
    return _1D::LinearInterpolator<double>();

  if( type == "spline" )
    return _1D::CubicSplineInterpolator<double>();

  if( type == "monotonic" )
    return _1D::MonotonicInterpolator<double>();

  throw std::runtime_error("Interpolator type was not recognized: '"+type+"'");
}



int main( int argc, char* argv[])
{

    po::options_description args("Arguments");
    args.add_options()
      ("interp-data" , "file containing data to be interpolated from.")
      ("x-values"    , "file containing x values to interpolate to.")
      ("output-file" , "file to write interpolated data to.");

    po::options_description opts("Options");
    opts.add_options()
      ("help,h"      , "print help message")
      ("batch,b"     , "output in 'batch' mode")
      ("method,m"    , po::value<string>()->default_value("spline"),     "interpolation method.")
      ("list,l"      ,                                                   "list available interpolation methods.")
      ("precision,p" , po::value<int>(),                                 "precision to use when writing output.")
      ;


    po::options_description opts_and_args("All Options");
    opts_and_args.add(opts).add(args);


    po::positional_options_description pos_args;
    pos_args.add("interp-data", 1);
    pos_args.add("x-values", 1);
    pos_args.add("output-file", 1);

    po::variables_map  vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(opts_and_args)
                                    .positional(pos_args)
                                    .allow_unregistered()
                                    .run();
    po::store(parsed, vm);
    po::notify(vm);




    if (argc == 1 || vm.count("help"))
    {
      print_version();
      print_usage( argv[0] );
      cout<<"\n";
      cout << opts << "\n";
      cout<<"\n";
      print_documentation();
      cout<<"\n";
      return 1;
    }

    if (vm.count("list"))
    {
      print_version();
      cout<<"\tlinear\n";
      cout<<"\tspline\n";
      cout<<"\tmonotonic\n";
      return 1;
    }


    ifstream in;
    double *x, *y;
    int n;

    // load data
    in.open( vm["interp-data"].as<string>().c_str() );
    Utils::ReadFunction( in, x, y, n );
    in.close();

    _1D::AnyInterpolator<double> interp = create(vm["method"].as<string>());
    interp.setData(n,x,y);
    delete[] x;
    delete[] y;

    // load x values
    in.open( vm["x-values"].as<string>().c_str() );
    Utils::ReadFunction( in, x, y, n, 1, 0 ); // multiplicity 0, only 1 column with coordinates.
    in.close();

    for(int i = 0; i < n; i++)
      y[i] = interp(x[i]);


    // write interpolated data
    ofstream out;
    if(vm.count("precision"))
    {
      out.precision( vm["precision"].as<int>() );
    }
    out.open( vm["output-file"].as<string>().c_str() );
    for(int i = 0; i < n; i++)
      out << x[i] << " " <<  y[i] << "\n";
    out.close();

    
    delete[] x;
    delete[] y;

  
    return 0;
}

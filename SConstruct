import os

# Use current directory as root staging point.
rootStage = os.getcwd()

#Set default C++ building flags for both libraries and executables
commonEnv = Environment(ENV = os.environ)
commonEnv.PrependENVPath("PATH", ".")

# Place include files here
commonEnv.IncDest = rootStage + '/include'

# Files installed 
commonEnv.RootDest = rootStage + '/target'
commonEnv.LibDest = commonEnv.RootDest + '/lib'
commonEnv.BinDest = commonEnv.RootDest + '/bin'

# Add Include Destination to general include directories
# Compile with all warnings, no opti, debug
commonEnv.Append(CPPPATH = [commonEnv.IncDest])
commonEnv.Append(CFLAGS = ' -std=gnu11 -Wall -pedantic')
commonEnv.Append(CFLAGS = '-O2')
commonEnv.Append(CXXFLAGS = ' -std=gnu++11 -Wall -pedantic')
commonEnv.Append(CXXFLAGS = '-O2')
#commonEnv.Replace(LINK = 'ld')

# Set linking flags for executables
bin_env = commonEnv.Clone()
bin_env.Append(LIBPATH = [bin_env.LibDest])
bin_env.Append(LINKFLAGS = ['-Wl,-rpath,'+bin_env.LibDest])

# Set linking flags for libraries
lib_env = commonEnv.Clone()
lib_env.Append(LINKFLAGS = ['-L'+commonEnv.LibDest])

SConscript(Split('lib/SConscript'),\
	exports='bin_env lib_env', variant_dir='tmp/lib', duplicate=0)
SConscript(Split('prog/SConscript'),\
	exports='bin_env lib_env', variant_dir='tmp/prog', duplicate=0)

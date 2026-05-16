# enable_exceptions.py
Import("env")
env.Append(CXXFLAGS=["-fexceptions"])
env.Replace(CXXFLAGS=[f for f in env.get("CXXFLAGS") if f != "-fno-exceptions"])

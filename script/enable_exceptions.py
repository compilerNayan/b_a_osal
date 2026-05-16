# enable_exceptions.py
Import("env")

print(">>> [enable_exceptions.py] Running pre-build script...")

# Add -fexceptions
env.Append(CXXFLAGS=["-fexceptions"])
print(">>> [enable_exceptions.py] Added -fexceptions")

# Remove -fno-exceptions if present
old_flags = env.get("CXXFLAGS")
new_flags = [f for f in old_flags if f != "-fno-exceptions"]
if len(new_flags) != len(old_flags):
    print(">>> [enable_exceptions.py] Removed -fno-exceptions")
env.Replace(CXXFLAGS=new_flags)

print(">>> [enable_exceptions.py] Final CXXFLAGS:", env.get("CXXFLAGS"))

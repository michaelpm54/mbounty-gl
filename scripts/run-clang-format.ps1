if (-not ('../src' | Test-Path))
{
	throw "This script should be run from the scripts/ directory."
}

foreach($file in Get-ChildItem -Path ../src -recurse -include *.cpp,*.h,*.hpp)
{
    clang-format -style=file -i $file
}

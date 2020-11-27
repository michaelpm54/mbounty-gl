if [[ ! $(basename "$PWD") == "scripts" ]]; then
    echo "This script should be run from the scripts/ directory."
    exit 1
fi

find ../src -iname *.hpp -o -iname *.h -o -iname *.cpp | xargs clang-format -i -style=file --verbose

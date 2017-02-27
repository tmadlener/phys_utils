# small helper function for less typing effort
# execute command (all but first argument) only if first argument
# contains of only '1's concatenated with (single) '+'s
function condExecute() {
  if [[ ${1} =~ ^(1\+)?1$ ]]; then
    shift
    $@
  fi
}

## produce a pdf from the passed .tex file, clean up afterwards and return to the
## directory the script was in before this was called
function cleanLatex() {
  cwd=$(pwd)
  cd $(dirname ${1})
  fn=$(basename ${1})
  basetex=${fn%.*}
  pdflatex ${fn} -interaction=nonstopmode
  # pdflatex ${1} -interaction=nonstopmode # run twice for correct references

  rm ${basetex}.{log,aux}
  cd ${cwd}
}


## parse the passed arguments for 'plot', 'create' and 'fit' and set the uppercased
## variable to 1 if found. 'all' sets all variables to 1
## variables set in this way can be parsed by the condExecute if they are concatenated with a '+'
function parseArgs() {
  for arg in "$@"; do
    case ${arg} in
    all )
      CREATE=1
      PLOT=1
      FIT=1
      ;;
    plot )
      PLOT=1
      ;;
    create )
      CREATE=1
      ;;
    fit )
      FIT=1
      ;;
    esac
  done
}

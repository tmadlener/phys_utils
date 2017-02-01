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

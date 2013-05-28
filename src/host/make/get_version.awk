$1 == "#define" && $2 == "STRETCH_MAJOR_VERSION"     { major    = $3 }
$1 == "#define" && $2 == "STRETCH_MINOR_VERSION"     { minor    = $3 }
$1 == "#define" && $2 == "STRETCH_REVISION_VERSION"  { revision = $3 }
END {
    print major " " minor " " revision
}

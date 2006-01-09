BEGIN {
  if (ARGC != 3) {
    print "genconf.awk: Usage 'awk -f genconf.awk dir file'" | "cat 1>&2"
    exit 1
  }
  level = 0
  dir = ARGV[1]
  file[level] = dir "/" ARGV[2]
  delete ARGV[1]
  delete ARGV[2]

  for (;;) {
    if ((getline data < file[level]) <= 0) {
      # file end, step up one level
      if (!level)
        # original file, all done
        break;
#      comps = split(file[level], comp, "/")
#      print "# done with: " comp[comps]
      --level
      continue
    }
    # replace all whitespace with a single space
    gsub(/[ \t]+/, " ", data)
    if (data ~ /^[.]include /) {
      # hanlde .include directive
      file[++level] = substr (data, 10)
      if (sub(/^@ggi_sysconfdir@\/targets\//, dir "/display/", file[level])) {
        # target specific config, look target subdir beneath display dir
        comps = split(file[level], comp, "/")
        sub(/[^.]+/, "&/&", comp[comps])
        file[level] = comp[1]
        for (i = 2; i <= comps; ++i)
          file[level] = file[level] "/" comp[i]
      }
      sub(/^@ggi_sysconfdir@\//, dir "/", file[level])
      # slap on ".in"
      file[level] = file[level] ".in"
#      comps = split(file[level], comp, "/")
#      print "# including: " comp[comps]
      continue
    }
    char = substr(data, 1, 1)
    # ignore directives
    if (char == ".")
      continue
    # clear out references to dynamic modules
    if (data ~ /@DLLEXT@/)
      continue
    print "\"" data "\","
  }
}	

for f in dist/*.msi;
do
  gpg --sign --detach --armor $f
done

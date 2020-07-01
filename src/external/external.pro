TEMPLATE = subdirs

SUBDIRS += botan2 qssh limereport
qssh.depends = botan2

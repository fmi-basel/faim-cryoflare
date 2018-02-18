TEMPLATE  = subdirs

 
SUBDIRS += \
           mrcio  \
           app
 
mrcio.subdir  = mrcio
app.subdir  = app

app.depends = mrcio
  

#------------------------------------------------------------------------------
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http:#www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------------
import numpy as np
import sys
import io
import time

class StarTable:
    def __init__(self):
        self.columns=[]
        self.data=None

    def readData(self,data):
        self.columns=[]
        self.data=None
        while data and not data[0].rstrip():
            data.pop(0)
        if data[0].startswith("loop_"):
            while data and not data[0].startswith("_rln"):
                data.pop(0)
            while data and data[0].startswith("_rln"):
                self.columns.append(data.pop(0)[4:].split(" ")[0].rstrip("\n"))
            self.data=np.array([],dtype=object).reshape(0,self.numColumns())
            rows=[]
            while data and data[0].rstrip():
                #rows.append(data.pop(0))
                sp=data.pop(0).split()
                if len(sp)!=len(self.columns):
                    raise Exception("column mismatch: expected %d columns but found %d columns in line: %s" % (len(self.columns),len(sp),sp) )
                rows.append(np.array(sp,dtype=object).reshape(1,self.numColumns()))
            self.data=np.concatenate(rows,axis=0)
            #self.data=np.loadtxt(io.StringIO("".join(rows)), dtype=str).astype(object)
        else:
            while not data[0].startswith("_rln"):
                data.pop(0)
            values=[]
            while data[0].startswith("_rln"):
                sp=data.pop(0)[4:].split(" ")[0:2]
                self.columns.append(sp[0])
                values.append(sp[1])
            self.data=np.array(values,dtype=object).reshape(1,self.numColumns())

    def writeData(self):
        output=""
        if self.numRows()>0:
            output+="loop_\n"
            for i in range(self.numColumns()):
                output+="_rln%s #%d \n" % (self.columns[i],i+1)
            bytes_io=io.BytesIO()
            np.savetxt(bytes_io, self.data,fmt="%s")
            output+=bytes_io.getvalue().decode('latin1')
        else:
            for i in range(self.numColumns()):
                output+="_rln"+self.columns[i]+" "+self.data[column]+"\n"
        return output

    def numColumns(self):
        return len(self.columns)

    def numRows(self):
        return self.data.shape[0]
    
    def addColumn(self,name,default_val):
        self.columns.append(name)
        self.data=np.concatenate((self.data,np.full((self.numRows(),1),str(default_val),dtype=object)),axis=1)
    
    def removeColumn(self,name):
        col_idx=self.columns.index(name)
        del self.columns[col_idx]
        self.data=np.delete(self.data,col_idx,1)

        

    def addRow(self,values):
        if self.numColumns()!=len(values):
            raise Exception("Number of values (%d) doesn't match number of columns (%d) " % (len(values),self.numColumns()))
        self.data=np.concatenate((self.data,np.array(list(map(str,values)),dtype=object).reshape(1,self.numColumns())),axis=0)

    def removeRow(self,idx):
        self.data=np.delete(self.data,idx,0)


    def __getitem__(self,key):
        # return single value (e.g. row 4 defocus X value [4,"DefocusX"] )
        if isinstance(key, tuple) and len(key)==2 and isinstance(key[0], int) and isinstance(key[1], str):
            return self.data[key[0],self.columns.index(key[1])]
        else:
            # return single row  (e.g. row 4  [4] ) or multiple rows (e.g. rows 4-8  [4:9]  or rows 4 and 6 [[4,6]])
            if isinstance(key, int) or  isinstance(key, slice) or  isinstance(key, list):
                key=(key,self.columns)
            # return single column (e.g. defocus X column ["DefocusX"] )
            if isinstance(key,str):
                key=(slice(None),key)
            # complex selection with single column
            if isinstance(key,tuple) and isinstance(key[1], str):
                key=(key[0],[key[1]])
            # otherwise return subset of columns and rows (e.g. [ 11:15,["DefocusX",DefocusY] ])
            column_ids=[self.columns.index(name) for name in key[1]]
            result=StarTable()
            if isinstance(key[0], list):
                result.data=self.data[np.array(key[0])[:, np.newaxis],column_ids]
            else:
                result.data=self.data[key[0],column_ids]
            result.columns=key[1]
            return result

    def __setitem__(self,key,value):
        if not isinstance(key, tuple) and len(key)==2:
            raise KeyError("Key should be an int/string pair")
        if isinstance(key[0], int) and isinstance(key[1], str):   
            self.data[key[0],self.columns.index(key[1])]=str(value)
        else:
            raise KeyError("Key should be an int/string pair")


class StarFile:
    def __init__(self,filename=""):
        self.tables={}
        if filename:
            self.read(filename)
        else:
            self.filename=""

    def read(self,filename):
        self.filename=filename
        with open(filename) as f:
            lines=f.readlines()
        while lines:
            if lines[0].startswith("data_"):
                key=lines.pop(0).rstrip()
                table=StarTable()
                table.readData(lines)
                self.tables[key]=table
            else:
                lines.pop(0)

    def write(self,filename=""):
        output=""
        for key in self.tables:
            output+="\n"+key+"\n\n"
            output+=self.tables[key].writeData()+"\n"
        if not filename:
            filename=self.filename
        with open(filename,"w") as f:
            f.write(output)

    def addTable(self,name,table=StarTable()):
        self.tables[name]=table
    
    def removeTable(self,name):
        del self.tables[name]

    def __getitem__(self, key):
        return self.tables[key]

    def __setitem__(self,key,table):
        self.tables[key]=table           


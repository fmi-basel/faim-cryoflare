from pystar import *
# load star file
star_file=StarFile("run_data.star")
# get a table from star file
data_table=star_file["data_"]
# get/set single value in table (DefocusX value in 5th row of table in this example)
print(data_table[5,"DefocusU"])
data_table[5,"DefocusU"]=-15000
# select subset of data from table
defocusX_column=data_table["DefocusU"]
rows_128_to_150=data_table[2:5]
# multiple columns and rows
complex_selection=data_table[2:5,["DefocusU","DefocusV"]]

# add column with column name and default value for all rows
complex_selection.addColumn("MyValue",33)
#column labels are stored in .columns
print(complex_selection.columns)
# data is stored in .data
print(complex_selection.data)
# add row 
complex_selection.addRow([15000,17000,21])
#create new star file and add table
new_star_file=StarFile()
new_star_file.addTable("data_selected",complex_selection)
new_star_file.write("run_data_selected.star")
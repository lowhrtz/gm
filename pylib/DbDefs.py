# -*- coding: utf-8 -*-
from itertools import izip

class Table(object):
    table_name = "Test"
    cols = None
    colDefs = None
    display_col = 0
    data = None
    
    def __str__(self):
        return self.table_name
        
    def get_cols(self):
        return self.cols

    def get_colDefs(self):
        return self.colDefs
        
    def get_display_col(self):
        return self.display_col

    def get_data(self):
        return self.data

    def get_table(self):
        record_list = []
        for record in self.get_data():
            record_list.append(dict(izip(self.get_cols(), record)))
        return record_list

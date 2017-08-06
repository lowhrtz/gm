# -*- coding: utf-8 -*-

class Manage( object ):
    enabled = True
    menu_list = []
    action_list = []
    connect_list = []
    widget_matrix = []

    def __str__( self ):
        return 'Manage'

    def add_menu( self, menu ):
        self.menu_list.append( menu )

    def add_action( self, action ):
        self.action_list.append( action )

#    def connect( self, trigger_widget, triggered_widget, callback ):
#        self.connect_list.append( ( trigger_widget, triggered_widget, callback ) )

    def add_row( self, widget_list ):
        self.widget_matrix.append( widget_list )

    def get_menu_list( self ):
        return self.menu_list

    def get_action_list( self ):
        return self.action_list

#    def get_connect_list( self ):
#        return self.connect_list

    def get_widget_matrix( self ):
        return self.widget_matrix


class Widget( object ):
    field_name = None
    widget_type = None
    enable_edit = True
    height = None
    width = None
    col_span = 1
    row_span = 1
    align = None
    data = None

    def __init__( self, field_name, widget_type, enable_edit=True, height=None, width=None, col_span=1, row_span=1, align=None, data=None ):
        self.field_name = field_name
        self.widget_type = widget_type
        self.enable_edit = enable_edit
        self.height = height
        self.width = width
        self.col_span = col_span
        self.row_span = row_span
        self.align = align
        self.data = data

    def __str__( self ):
        return self.field_name

    def get_field_name( self ):
        return self.field_name

    def get_widget_type( self ):
        return self.widget_type

    def is_edit_enabled( self ):
        return self.enable_edit

    def get_height( self ):
        return self.height

    def get_width( self ):
        return self.width

    def get_col_span( self ):
        return self.col_span

    def get_row_span( self ):
        return self.row_span

    def get_align( self ):
        return self.align

    def get_data( self ):
        return self.data


class Action( object ):
    def __init__( self, action_type, widget1=None, widget2=None, callback=None ):
        self.action_type = action_type
        self.widget1 = widget1
        self.widget2 = widget2
        self.callback = callback


class Menu( object ):
    def __init__( self, menu_name ):
        self.menu_name = menu_name
        self.action_list = []

    def get_menu_name( self ):
        return self.menu_name

    def add_action( self, action ):
        self.action_list.append( action )

    def get_action_list( self ):
        return self.action_list

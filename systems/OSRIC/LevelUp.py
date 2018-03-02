#!/usr/bin/env python
# -*- coding: utf-8 -*-

import DbQuery
from GuiDefs import *


class LevelUpWizard( Wizard ):
    def __init__( self ):
        super( LevelUpWizard, self ).__init__( 'Level Up' )

        self.add_wizard_page( IntroPage() )
        self.add_wizard_page( SpellbookPage() )

        self.add_wizard_page( ProficiencyPage() )
        self.add_wizard_page( ReviewPage() )


class IntroPage( WizardPage ):
    def __init__( self ):
        super( IntroPage, self ).__init__( 0, 'Level Up' )
        self.set_subtitle( 'Level-up Wizard' )

        text = Widget( 'Intro Text', 'TextLabel', align='Center' , data='There are one or more classes ready to level up!' )
        self.add_row( [ text ] )

        text2 = Widget( 'Intro Text2', 'TextLabel', align='Center', data='Click <b>Next</b> to continue.' )
        self.add_row( [ text2 ] )


    def initialize_page( self, fields, pages, external_data ):
        pc = external_data['Character List Current']
        classes = pc['Classes'].split( '/' )
        level = str( pc['Level'] ).split( '/' )
        self.ready_list = ready_to_level_up( pc )
        self.ready_dict_list = [ cl for cl in DbQuery.getTable( 'Classes' ) if cl['unique_id'] in self.ready_list ]
        wizard_category = False
        priest_category = False
        self.proficiency_slots_available = 0
        for cl in self.ready_dict_list:
            if cl['Category'] == 'wizard':
                wizard_category = True
            wpa = cl['Weapon_Proficiency_Advancement'].split('/')
            slots = int( wpa[0] )
            per_level = int( wpa[1] )
            pc_level = int( level[ classes.index( cl['unique_id'] ) ] )
            if pc_level % per_level == 0:
                self.proficiency_slots_available = slots

        if wizard_category:
            self.next_page_id = pages['Spellbook'].get_page_id()

        elif self.proficiency_slots_available:
            self.next_page_id = pages['Proficiencies'].get_page_id()

        elif self.ready_list:
            self.next_page_id = pages['Review'].get_page_id()

        else:
            self.next_page_id = -1
            return { 'Intro Text': 'There are no classes ready to level up!',
                     'Intro Text2': '' }

    def get_next_page_id( self, fields, pages, external_data ):
        return self.next_page_id


class SpellbookPage( WizardPage ):
    def __init__( self ):
        super( SpellbookPage, self ).__init__( 1, 'Spellbook' )
        self.set_subtitle( 'Choose a spell to add to your spellbook' )

        name = Widget( 'Name', 'LineEdit' )

        self.add_row( [ name ] )

    def initialize_page( self, fields, pages, external_data ):
        pass

    def get_next_page_id( self, fields, pages, external_data ):
        return -1

class ProficiencyPage( WizardPage ):
    def __init__( self ):
        super( ProficiencyPage, self ).__init__( 4, 'Proficiencies' )
        self.set_subtitle( 'Choose available proficiencies' )

    def initialize_page( self, fields, pages, external_data ):
        print pages['Level Up'].ready_list


class ReviewPage( WizardPage ):
    def __init__( self ):
        super( ReviewPage, self ).__init__( 5, 'Review' )
        self.set_subtitle( 'Make sure you like what you see' )

        name = Widget( 'Name', 'LineEdit' )

        self.add_row( [ name, ] )

    def is_complete( self, fields, pages, external_data ):
        if fields['Name'] == '' or fields['Name'].isspace():
            return False
        else:
            return True


def ready_to_level_up( pc ):
    classes_meta = DbQuery.getTable( 'Classes_meta' )
    classes = pc['Classes'].split( '/' )
    xp = str( pc['XP'] ).split( '/' )
    level = str( pc['Level'] ).split( '/' )
    level_up_classes = []
    for i, cl in enumerate( classes ):
        cl_xp = int( xp[i] )
        cl_level = int( level[i] )
        xp_table = [ row for row in classes_meta if row['class_id'] == cl and row['Type'] == 'xp table' ]
        each = None
        for j, row in enumerate( xp_table ):
            if row['Level'].lower() == 'each':
                each = j
        if each is not None:
            each = xp_table.pop( each )

        xp_table.sort( key=lambda x: int( x['Level'] ) )
        top_row = xp_table[ len( xp_table ) - 1 ]
        top_row_level = int( top_row['Level'] )
        if cl_level > top_row_level:
            levels_past_top_row = ( cl_xp - top_row['XP'] ) // each['XP']
            if top_row_level + levels_past_top_row > cl_level:
                level_up_classes.append( cl )
        else:
            for j, row in enumerate( xp_table ):
                if int( row['Level'] ) == cl_level + 1:
                    if cl_xp >= row['XP']:
                        level_up_classes.append( cl )
                    break

    return level_up_classes

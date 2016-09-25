# -*- coding: utf-8 -*-
from WizardDefs import WizardPage
import SystemSettings

def get_pc_gender_list():
    genList = []
    for gender in SystemSettings.gender:
        if(gender != 'NA'):
            genList.append(gender)

    return genList

def get_attribute_names():
    attrList = []
    for attribute in SystemSettings.attributes:
        attrList.append(attribute[0])

    return attrList

class IntroPage(WizardPage):
    page_title = "Intro"
    page_subtitle = "Intro Subtitle"
    page_id = 0
    content = "This is the wizard for creating your character."

class CharacterCreationChoicePage(WizardPage):
    page_title = "You Must Choose!"
    page_subtitle = "But choose wisely..."
    page_id = 10
    template = "infoPage"
    content = [
        ('text', 'How do you want to create your character?'),
        ('fillHook', 'get_choices', None, 'radiobutton', 'index'),
    ]

    def get_choices(self):
        choices = [
            'Roll Attributes First',
            'Choose Race, Class, etc. First',
        ]
        return choices

    def get_next_page_id(self, selected_radio=0):
        if selected_radio == 0:
            return RollMethodsPage.page_id
        else:
            return InfoPage.page_id

class RollMethodsPage(WizardPage):
    page_title = "Roll Methods"
    page_subtitle = "Choose your roll method"
    page_id = 20
    template = "rollMethodsPage"
    attribute_list = get_attribute_names()
    content = [
        ('classic','3d6'),
        ('classic+arrange','3d6'),
        ('droplow','4d6'),
        ('droplow+arrange','4d6'),
    ]

#    def get_next_page_id(self):
#        return InfoPage.page_id

#class RollAttributesPage(WizardPage):
#    page_title = "Roll"
#    page_subtitle = "Roll for you character's attributes."
#    page_id = 3
#    template = "infoPage"
#    content = [
#        ('text','Roll your attributes.'),
#        ('fillHook','get_attributes',None,'spinbox',True),
#        ('buTTon','Reroll','roll_action'),
#        ('text','Warning: clicking "Back" will reset the scores.'),
#    ]

#    def get_attributes(self):
#        attrList = []
#        for attribute in SystemSettings.attributes:
#            attrList.append(attribute[0])

#        return attrList

#    def roll_action(self):
#        """Returns a tuple, details TBD"""
#        return ('Dice.rollDice','3d6', self.get_attributes())

class InfoPage(WizardPage):
    page_title = "Information"
    page_subtitle = "Please enter personal information for your character."
    page_id = 40
    template = "InfoPage"
    content = [
        ('text','This is the basic information that you will build your character on.'),
        ('field','Name',True),
        ('choose','Alignment','Settings','alignment'),
        ('choose','Race','DB','Races'),
        ('choose','Gender','Function',get_pc_gender_list),
        ('choose','Class','DB','Classes'),
#        ('checkbox-checked', 'This is a spellcaster', 'bool'),
    ]

#    def get_next_page_id(self, checked=True):
#        if checked:
#            return 5
#        else:
#            return 6

class ChooseClassPage(WizardPage):
    page_title = "Choose Class"
    page_subtitle = "Choose Class Subtitle"
    page_id = 50
    layout = "Horizontal"
    content = "This is where you'd choose a class."

class ReviewPage(WizardPage):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    page_id = 60
    content = "This is the review content."

#    def get_next_page_id(self):
#        return -1

#page_order = [
#    IntroPage,
#    CharacterCreationChoicePage,
#    RollMethodsPage,
#    #RollAttributesPage,
#    InfoPage,
#    ChooseClassPage,
#    ReviewPage,
#    ]

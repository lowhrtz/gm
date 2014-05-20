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
    content = "This is the wizard for creating your character."

class RollMethodsPage(WizardPage):
    page_title = "Roll Methods"
    page_subtitle = "Choose your roll method"
    template = "rollMethodsPage"
    attribute_list = get_attribute_names()
    content = [
        ('classic','3d6'),
        ('classic+arrange','3d6'),
        ('droplow','4d6'),
        ('droplow+arrange','4d6'),
    ]

class RollAttributesPage(WizardPage):
    page_title = "Roll"
    page_subtitle = "Roll for you character's attributes."
    template = "infoPage"
    content = [
        ('text','Roll your attributes.'),
        ('fillHook','get_attributes',None,'spinbox',True),
        ('buTTon','Reroll','roll_action'),
        ('text','Warning: clicking "Back" will reset the scores.'),
    ]

    def get_attributes(self):
        attrList = []
        for attribute in SystemSettings.attributes:
            attrList.append(attribute[0])

        return attrList

    def roll_action(self):
        """Returns a tuple, details TBD"""
        return ('Dice.rollDice','3d6', self.get_attributes())

class InfoPage(WizardPage):
    page_title = "Information"
    page_subtitle = "Please enter personal information for your character."
    template = "InfoPage"
    content = [
        ('text','This is the basic information that you will build your character on.'),
        ('field','Name',True),
        ('choose','Alignment','Settings','alignment'),
        ('choose','Race','DB','Races'),
        ('choose','Gender','Function',get_pc_gender_list),
        ('choose','Class','DB','Classes'),
    ]

class ChooseClassPage(WizardPage):
    page_title = "Choose Class"
    page_subtitle = "Choose Class Subtitle"
    layout = "Horizontal"
    content = "This is where you'd choose a class."

class ReviewPage(WizardPage):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    content = "This is the review content."

page_order = [
    IntroPage,
    RollMethodsPage,
    #RollAttributesPage,
    InfoPage,
    ChooseClassPage,
    ReviewPage,
    ]

#include <hildon/hildon.h>
#include "ballistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
        COL_RANGE = 0,
        COL_DROP,
        COL_DROP_M,
        COL_DRIFT,
        COL_DRIFT_M,
        COL_VELOCITY,
        NUM_COLS
};

#include "calcSerializer.c"

// change column names from MOA to Mils if the second argument is true
// this lets us create the treeview structures once and just update
// them as necessary
void fixColNames (GtkTreeView *ballHeaderView, int isMils) {
        GtkTreeViewColumn *tCol;

        tCol = gtk_tree_view_get_column(ballHeaderView,2);
        gtk_tree_view_column_set_title(tCol, (isMils?"Y Mils":"Y MOA"));

        tCol = gtk_tree_view_get_column(ballHeaderView,4);
        gtk_tree_view_column_set_title(tCol, (isMils?"X Mils":"X MOA"));
}

// create a new tree view for the ballistics data
GtkWidget *makeBallColumns (void) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *tCol;
        GtkWidget *ballView;

        ballView = gtk_tree_view_new();

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "Range", renderer, "text", COL_RANGE, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "Drop", renderer, "text", COL_DROP, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "Y MOA" , renderer, "text", COL_DROP_M, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "Drift", renderer, "text", COL_DRIFT, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "X MOA", renderer, "text", COL_DRIFT_M, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        (GtkTreeView *) ballView, -1, "Velocity", renderer, "text", COL_VELOCITY, NULL);

        int i;
        for (i=0;i<NUM_COLS;i++) {
                tCol = gtk_tree_view_get_column((GtkTreeView *) ballView, i);
                gtk_tree_view_column_set_fixed_width(tCol,130);
                gtk_tree_view_column_set_sizing(tCol, GTK_TREE_VIEW_COLUMN_FIXED);
        }

        return ballView;
}

void pack2Horizontal (GtkWidget **field1, char *name1, GtkWidget **field2, char *name2, GtkWidget **vbox)
{
        // this used to pack two fields horizontally, but that doesn't look good on the phone, so
        // now it just puts two fields in, one after the other.

        GtkWidget *tempHbox;

        tempHbox = gtk_hbox_new(TRUE,0);
        *field1 = hildon_entry_new( HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT );
        hildon_entry_set_placeholder( (HildonEntry *) *field1, name1 );
        gtk_box_pack_start((GtkBox *) tempHbox, gtk_label_new(name1), FALSE, FALSE, 0);
        gtk_box_pack_start((GtkBox *) tempHbox, *field1, FALSE, FALSE, 0);

        gtk_box_pack_start((GtkBox *) *vbox, tempHbox, TRUE, TRUE, 0);

        if (field2 != NULL) {
                tempHbox = gtk_hbox_new(TRUE,0);
                *field2 = hildon_entry_new( HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT );
                hildon_entry_set_placeholder( (HildonEntry *) *field2, name2 );
                gtk_box_pack_start((GtkBox *) tempHbox, gtk_label_new(name2), FALSE, FALSE, 0);
                gtk_box_pack_start((GtkBox *) tempHbox, *field2, FALSE, FALSE, 0);

                gtk_box_pack_start((GtkBox *) *vbox, tempHbox, TRUE, TRUE, 0);
        }

        return;
}

void makeMainWindow (GtkWidget **window, GtkWidget **button, GtkWidget **nameField,
                GtkWidget **bcField, GtkWidget **vField, GtkWidget **shField,
                GtkWidget **angField, GtkWidget **zeroField, GtkWidget **wsField,
                GtkWidget **waField, GtkWidget **altField, GtkWidget **pressField,
                GtkWidget **temprField, GtkWidget **humField, GtkWidget **atmButton,
                GtkWidget **milButton, GtkWidget **dragSelButton, GtkWidget **rngField,
                GtkWidget **incField)
{
        GtkWidget *tempVbox;
        GtkWidget *tempHbox;
        GtkWidget *mainArea;
        GtkWidget *dragSelector;
        GtkWidget *milSelector;

                *window = hildon_stackable_window_new();
        gtk_window_set_title ( (GtkWindow *) *window, "BC" );

        *button = hildon_button_new_with_text 
                (HILDON_SIZE_HALFSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
                 HILDON_BUTTON_ARRANGEMENT_HORIZONTAL, 
                 "Compute", NULL);

        tempVbox = gtk_vbox_new(TRUE, 0);
        // button goes on last line of vbox
        gtk_box_pack_end((GtkBox *) tempVbox, *button, TRUE, TRUE, 0);
        
        // line 1: name, zero range
        pack2Horizontal(nameField, "Name", zeroField, "Zero Range (yards)", &tempVbox);
        // line 2: BC, velocity
        pack2Horizontal(bcField, "BC", vField, "Velocity (fps)", &tempVbox);
        // line 3: sh, angle
        pack2Horizontal(shField, "Sight Height (inches)", angField, "Sight Angle (degrees)", &tempVbox);
        // line 4: wind speed, wind angle
        pack2Horizontal(wsField, "Wind Speed (MPH)", waField, "Wind Angle (degrees)", &tempVbox);
        // line 5: max range and increment
        pack2Horizontal(rngField, "Max Range (yards)", incField, "Range Increment (yards)", &tempVbox);
        // line 6: option buttons
        tempHbox = gtk_hbox_new(TRUE,0);
        
        // mil/MOA selector
        milSelector = hildon_touch_selector_new_text();
        hildon_touch_selector_append_text( (HildonTouchSelector *) milSelector, "MOA" );
        hildon_touch_selector_append_text( (HildonTouchSelector *) milSelector, "Mils" );
        hildon_touch_selector_set_column_selection_mode( (HildonTouchSelector *) milSelector, HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE );
        // button for mil/MOA selector
        *milButton = hildon_picker_button_new(HILDON_SIZE_AUTO, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
        hildon_picker_button_set_selector( (HildonPickerButton *) *milButton, (HildonTouchSelector *) milSelector );
        hildon_picker_button_set_active( (HildonPickerButton *) *milButton, 0 );

        // drag model selector
        dragSelector = hildon_touch_selector_new_text();
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G1");
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G2");
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G5");
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G6");
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G7");
        hildon_touch_selector_append_text( (HildonTouchSelector *) dragSelector, "G8");
        hildon_touch_selector_set_column_selection_mode( (HildonTouchSelector *) dragSelector, HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE );
        // button for drag model selector
        *dragSelButton = hildon_picker_button_new(HILDON_SIZE_AUTO, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
        hildon_picker_button_set_selector( (HildonPickerButton *) *dragSelButton, (HildonTouchSelector *) dragSelector );
        hildon_picker_button_set_active( (HildonPickerButton *) *dragSelButton, 0 );

        // toggle button for atmospheric corrections
        *atmButton = gtk_toggle_button_new_with_label("Use Atmospheric Corrections");

        gtk_box_pack_start((GtkBox *) tempHbox, *milButton, TRUE, TRUE, 0);
        gtk_box_pack_start((GtkBox *) tempHbox, *dragSelButton, TRUE, TRUE, 0);

        gtk_box_pack_start((GtkBox *) tempVbox, tempHbox, TRUE, TRUE, 0);
        gtk_box_pack_start((GtkBox *) tempVbox, *atmButton, TRUE, TRUE, 0);
        // line 7: altitude, pressure
        pack2Horizontal(altField, "Altitude (feet)", pressField, "Pressure (inches Hg)", &tempVbox);
        // line 8: temperature, humidity
        pack2Horizontal(temprField, "Temperature (F)", humField, "Humidity (percent)", &tempVbox);

        // scrolling area
        mainArea = hildon_pannable_area_new_full ( HILDON_PANNABLE_AREA_MODE_AUTO,
                        TRUE, 10, 500, 0.93, 20 );
        hildon_pannable_area_add_with_viewport( (HildonPannableArea *) mainArea, tempVbox );

        // use a vbox as the top level structure to force the proper width
        tempVbox = gtk_vbox_new(TRUE,0);
        gtk_box_pack_start((GtkBox *) tempVbox, mainArea, TRUE, TRUE, 0);

        // assemble the window
        gtk_container_add( (GtkContainer *) *window, tempVbox );

        return;
}

void makeChildWindow (GtkWidget **childWindow, GtkWidget **ballDataView, GtkWidget **ballHeaderView)
{
        GtkTreeModel *ballHeaderModel;
        GtkWidget *ballDataArea;
        GtkWidget *childVbox;

        /* Create the subwindow */
        *childWindow = hildon_stackable_window_new();
        gtk_window_set_title( (GtkWindow *) *childWindow, "BC Solution" );
        *ballDataView = makeBallColumns();

        // non-scrolling header
        *ballHeaderView = makeBallColumns();
        gtk_tree_view_set_headers_visible ( (GtkTreeView *) *ballHeaderView, TRUE );
        ballHeaderModel = (GtkTreeModel *) gtk_list_store_new (
                        NUM_COLS, G_TYPE_INT, G_TYPE_FLOAT,
                        G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT,
                        G_TYPE_INT );
        gtk_tree_view_set_model( (GtkTreeView *) *ballHeaderView, ballHeaderModel );
        g_object_unref( ballHeaderModel );

        // scrolling area
        ballDataArea = hildon_pannable_area_new_full ( HILDON_PANNABLE_AREA_MODE_AUTO,
                        TRUE, 10, 500, 0.93, 20 );
        hildon_pannable_area_add_with_viewport( (HildonPannableArea *) ballDataArea, *ballDataView );

        // vbox
        childVbox = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start((GtkBox *) childVbox, *ballHeaderView, FALSE, FALSE, 0);
        gtk_box_pack_start((GtkBox *) childVbox, ballDataArea, TRUE, TRUE, 0);

        // assemble vbox into child window
        gtk_container_add( (GtkContainer *) *childWindow, childVbox );

        return;
}


int main (int argc, char **argv)
{
        // GUI stuff
        HildonProgram *program;
        GtkWidget *window, *button, *nameField, *bcField, *vField, *shField, *angField, *zeroField, *wsField, *waField, *altField, *pressField, *temprField, *humField, *atmButton, *milButton, *dragSelButton, *rngField, *incField;
        GtkWidget *childWindow, *ballDataView, *ballHeaderView;
        GtkWidget *boundsErrorNote;
        GtkTreeModel *ballDataModel;
        GtkTreeIter ballDataIter;

        // BC stuff
        int k, s;
        double *sln;
        double bc=0.625;
        double v=2950;
        double sh=1.6;
        double angle=0;
        double zero=100;
        double windspeed=0;
        double windangle=0;
        double zeroangle=0;
        double altitude;
        double pressure;
        double temperature;
        double humidity;
        double range;
        double increment;
        int dragFunction;
        int useMils;

        // program initialization
        g_set_application_name ("BC");
        hildon_gtk_init (&argc, &argv);
        program = hildon_program_get_instance ();

        // create main window
        makeMainWindow( &window, &button, &nameField, &bcField, &vField, &shField, &angField, &zeroField, &wsField, &waField, &altField, &pressField, &temprField, &humField, &atmButton, &milButton, &dragSelButton, &rngField, &incField );
        hildon_program_add_window (program, HILDON_WINDOW (window));
        g_signal_connect (G_OBJECT (window), "delete_event", (GCallback) gtk_main_quit, NULL);

        // create child window
        makeChildWindow (&childWindow, &ballDataView, &ballHeaderView);
        // closure!
        g_signal_connect ( (GObject *) childWindow, "delete_event", (GCallback) gtk_widget_hide, (gpointer) childWindow );


        void reportNum (GtkWidget *widget, gpointer data) {
                // read out data from the appropriate widgets
                bc = strtod( (char *) hildon_entry_get_text( (HildonEntry *) bcField ), NULL );
                v = strtod( (char *) hildon_entry_get_text( (HildonEntry *) vField ), NULL );
                sh = strtod( (char *) hildon_entry_get_text( (HildonEntry *) shField ), NULL );
                angle = strtod( (char *) hildon_entry_get_text( (HildonEntry *) angField ), NULL );
                zero = strtod( (char *) hildon_entry_get_text( (HildonEntry *) zeroField ), NULL );
                windspeed = strtod( (char *) hildon_entry_get_text( (HildonEntry *) wsField ), NULL );
                windangle = strtod( (char *) hildon_entry_get_text( (HildonEntry *) waField ), NULL );
                altitude = strtod( (char *) hildon_entry_get_text( (HildonEntry *) altField ), NULL );
                pressure = strtod( (char *) hildon_entry_get_text( (HildonEntry *) pressField ), NULL );
                temperature = strtod( (char *) hildon_entry_get_text( (HildonEntry *) temprField ), NULL );
                humidity = strtod( (char *) hildon_entry_get_text( (HildonEntry *) humField ), NULL );
                range = strtod( (char *) hildon_entry_get_text( (HildonEntry *) rngField ), NULL );
                increment = strtod( (char *) hildon_entry_get_text( (HildonEntry *) incField ), NULL );

                // bounds checking
                if (!bc || !zero || !v || !range || !increment) {
                        // can't get a good solution
                        boundsErrorNote = hildon_note_new_information( (GtkWindow *) window, "Error: BC, zero range, velocity, max range, and range increment must be nonzero." );
                        gtk_dialog_run( (GtkDialog *) boundsErrorNote );
                        gtk_object_destroy( (GtkObject *) boundsErrorNote );
                        return;
                }

                // if necessary, do atmospheric correction
                if (gtk_toggle_button_get_active( (GtkToggleButton *) atmButton )) {
                        if (!pressure) {
                                // nonzero pressure is kind of necessary
                                boundsErrorNote = hildon_note_new_information( (GtkWindow *) window, "Error: Pressure must be nonzero." );
                                gtk_dialog_run( (GtkDialog *) boundsErrorNote );
                                gtk_object_destroy( (GtkObject *) boundsErrorNote );
                                return;
                        }

                        bc = AtmCorrect(bc,altitude,pressure,temperature,humidity/100.0);
                }

                // convert from picker button notation to the drag function enum
                switch (hildon_picker_button_get_active( (HildonPickerButton *) dragSelButton )) {
                        case 0:
                                dragFunction = G1;
                                break;
                        case 1: 
                                dragFunction = G2;
                                break;
                        case 2:
                                dragFunction = G5;
                                break;
                        case 3:
                                dragFunction = G6;
                                break;
                        case 4:
                                dragFunction = G7;
                                break;
                        case 5:
                                dragFunction = G8;
                                break;
                        default:
                                dragFunction = G1;
                }

                // we could come back and add an option to set a height above true zero at specified zero range;
                // that would change the last argument to the ZeroAngle function. See ballistics.h for details.
                zeroangle = ZeroAngle(dragFunction,bc,v,sh,zero,0);

                k = SolveAll(dragFunction,bc,v,sh,angle,zeroangle,windspeed,windangle,&sln);

                ballDataModel = (GtkTreeModel *) gtk_list_store_new
                        (NUM_COLS, G_TYPE_INT, G_TYPE_FLOAT, G_TYPE_FLOAT,
                         G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_INT);

                useMils = hildon_picker_button_get_active( (HildonPickerButton *) milButton );

                for (s=0;(s<=k) && (s<=range);s+=increment) {
                        gtk_list_store_append ( (GtkListStore *) ballDataModel, &ballDataIter);
                        gtk_list_store_set ( (GtkListStore *) ballDataModel, &ballDataIter,
                                        COL_RANGE, (int) GetRange(sln,s),
                                        COL_DROP, GetPath(sln,s),
                                        COL_DROP_M, useMils ? 1000*MOAtoRad(GetMOA(sln,s)) : GetMOA(sln,s),
                                        COL_DRIFT, GetWindage(sln,s),
                                        COL_DRIFT_M, useMils ? 1000*MOAtoRad(GetMOA(sln,s)) : GetWindageMOA(sln,s),
                                        COL_VELOCITY, (int) GetVelocity(sln,s), -1);
                }
                // now that we've put the solution into the ballDataModel,
                // we don't need it any more
                free(sln);

                fixColNames((GtkTreeView *) ballHeaderView, useMils);

                gtk_tree_view_set_model ( (GtkTreeView *) ballDataView, ballDataModel );
                g_object_unref ( ballDataModel );

                gtk_widget_show_all(childWindow);
        }

        // connect the above function to the button press
        g_signal_connect (G_OBJECT (button), "clicked", (GCallback) reportNum, NULL);

        // show main window
        gtk_widget_show_all (GTK_WIDGET (window));

        // event loop
        gtk_main ();

        return 0;
}

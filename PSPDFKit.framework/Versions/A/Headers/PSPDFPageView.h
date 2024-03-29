//
//  PSPDFPageView.h
//  PSPDFKit
//
//  Copyright 2011-2012 Peter Steinberger. All rights reserved.
//

#import "PSPDFKitGlobal.h"
#import "PSPDFRenderQueue.h"

@protocol PSPDFAnnotationView;
@class PSPDFPageInfo, PSPDFScrollView, PSPDFDocument, PSPDFViewController, PSPDFTextParser, PSPDFTextSelectionView, PSPDFAnnotation, PSPDFRenderStatusView, PSPDFNoteAnnotation;

/// Send this event to hide any selections, menus or other interactive page elements.
extern NSString *const kPSPDFHidePageHUDElements;

/// Compound view for a single pdf page. Will not be re-used for different pages.
/// You can add your own views on top of the UIView (e.g. custom annotations)
/// Events from a attached UIScrollView will be relayed to PSPDFPageView's.
@interface PSPDFPageView : UIView <UIScrollViewDelegate, PSPDFRenderDelegate>

/// Designated initializer.
/// Note: We already need pdfController at this stage to check the classOverride table.
- (id)initWithFrame:(CGRect)frame pdfController:(PSPDFViewController *)pdfController;


/// @name Show / Destroy a document

/// configure page container with data.
- (void)displayDocument:(PSPDFDocument *)document page:(NSUInteger)page pageRect:(CGRect)pageRect scale:(CGFloat)scale delayPageAnnotations:(BOOL)delayPageAnnotations pdfController:(PSPDFViewController *)pdfController;

/// Prepares the PSPDFPageView for reuse. Removes all unknown internal UIViews.
- (void)prepareForReuse;


/// @name Internal views and rendering

/// Redraw the renderView
- (void)updateRenderView;

/// Redraw renderView and contentView.
- (void)updateView;

/// If annotations are already loaded, and the annotation is a view, access it here.
/// (Most PDF annotations are actually rendered into the page; except PSPDFLinkAnntotations and all derivates (video, etc) and PSPDFNoteAnnotation)
- (UIView<PSPDFAnnotationView> *)annotationViewForAnnotation:(PSPDFAnnotation *)annotation;

/// UIImageView subview showing the whole document. Readonly.
@property(nonatomic, strong, readonly) UIImageView *contentView;

/// UIImageView for the zoomed in state. Readonly.
@property(nonatomic, strong, readonly) UIImageView *renderView;

/// Size used for the zoomed-in part. Should always be bigger than the screen.
/// This is set to a good default already. You shound't need to touch this.
@property(nonatomic, assign) CGSize renderSize;

/// Calculated scale. Readonly.
@property(nonatomic, assign, readonly) CGFloat pdfScale;

/// Temporarily suspend rendering updates to the renderView.
@property(nonatomic, assign) BOOL suspendUpdate;

/// Is view currently rendering (either contentView or renderView)
@property(nonatomic, assign, getter=isRendering, readonly) BOOL rendering;

/// Current CGRect of the part of the page that's visible. Screen coordinate space.
/// Note: If the scrollview is currently decellerating and we're on iOS5 and upwards,
/// this will show the TARGET rect, not the one that's currently animating.
@property(nonatomic, assign, readonly) CGRect visibleRect;

/// Access the selectionView. (handles text selection)
@property(nonatomic, strong, readonly) PSPDFTextSelectionView *selectionView;

/// Access the render status view that is displayed on top of a page while we are rendering.
@property(nonatomic, strong) PSPDFRenderStatusView *renderStatusView;

/// Top right offset. Defaults to 30.
@property(nonatomic, assign) CGFloat renderStatusViewOffset;

/// Should center render status view. Defaults to NO.
@property(nonatomic, assign) BOOL centerRenderStatusView;

/// Shortcut to access the textParser corresponding to the current page.
@property(nonatomic, strong, readonly) PSPDFTextParser *textParser;


/// @name Coordinate calculations and object fetching

/// Convert a view point to the corresponding pdf point.
/// pageBounds usually is PSPDFPageView bounds.
- (CGPoint)convertViewPointToPDFPoint:(CGPoint)viewPoint;

/// Convert a pdf point to the corresponding view point.
/// pageBounds usually is PSPDFPageView bounds.
- (CGPoint)convertPDFPointToViewPoint:(CGPoint)pdfPoint;

/// Convert a view rect to the corresponding pdf rect.
- (CGRect)convertViewRectToPDFRect:(CGRect)viewRect;

/// Convert a pdf rect to the corresponding view rect
- (CGRect)convertPDFRectToViewRect:(CGRect)pdfRect;

/// Get the glyphs/words on a specific page.
- (NSDictionary *)objectsAtPoint:(CGPoint)pdfPoint options:(NSDictionary *)options;

/// Get the glyphs/words on a specific rect.
- (NSDictionary *)objectsAtRect:(CGRect)pdfRect options:(NSDictionary *)options;

/// @name Accessors

/// Access parent PSPDFScrollView if available. (zoom controller)
/// Note: this only lets you access the scrollView if it's in the view hiararchy.
/// If we use pageCurl mode, we have a global scrollView which can be accessed with pdfController.pagingScrollView
- (PSPDFScrollView *)scrollView;

/// Returns an array of UIView <PSPDFAnnotationView> objects currently in the view hierarchy.
- (NSArray *)visibleAnnotationViews;

/// Access pdfController
@property(nonatomic, ps_weak, readonly) PSPDFViewController *pdfController;

/// Page that is displayed. Readonly.
@property(atomic, assign, readonly) NSUInteger page;

/// Document that is displayed. Readonly.
@property(atomic, strong, readonly) PSPDFDocument *document;

/// Shortcut to access the current boxRect of the set page.
@property(nonatomic, assign, readonly) PSPDFPageInfo *pageInfo;

/// Return YES if the pdfPage is displayed in a double page mode setup on the right side.
@property(nonatomic, assign, readonly, getter=isRightPage) BOOL rightPage;


/// @name Shadow settings

/// Enables shadow for a single page. Only useful in combination with pageCurl.
@property(nonatomic, assign, getter=isShadowEnabled) BOOL shadowEnabled;

/// Set default shadowOpacity. Defaults to 0.7.
@property(nonatomic, assign) float shadowOpacity;

/// Subclass to change shadow behavior.
- (void)updateShadow;

/// Set block that is executed within updateShadow when isShadowEnabled = YES.
@property(nonatomic, copy) void(^updateShadowBlock)(PSPDFPageView *pageView);

@end


// Extensions to handle annotations.
@interface PSPDFPageView (PSPDFAnnotationMenu)

/// Currently selected annotation (selected by a tap; showing a menu)
@property(nonatomic, strong) PSPDFAnnotation *selectedAnnotation;

/**
    Hit-testing for a single PSPDFPage. This is usually a relayed event from the parent PSPDFScrollView.
    Returns YES if the tap has been handled, else NO.
    
    All annotations for the current page are loaded and hit-tested (except PSPDFAnnotationTypeLink; which has already been handled by now)
    
    If an annotation has been hit (via [annotation hitTest:tapPoint]; convert the tapPoint in PDF coordinate space via convertViewPointToPDFPoint) then we call showMenuForAnnotation.
 
    If the tap didn't hit an annotation but we are showing a UIMenuController menu; we hide that and set the touch as processed.
 */
- (BOOL)singleTapped:(UITapGestureRecognizer *)recognizer;

/// Handle long press, potentially relay to subviews.
- (BOOL)longPress:(UILongPressGestureRecognizer *)recognizer;

/// Returns available UIMenuItem for the current annotation.
/// To extend this, selectors for UIMenuItem need to be implemented in a subclass.
- (NSArray *)menuItemsForAnnotation:(PSPDFAnnotation *)annotation;

/// Called when a annotation was found ad the tapped location.
/// This will usually call menuItemsForAnnotation to show a UIMenuController,
/// Except for PSPDFAnnotationTypeNote which is handled differently (showNoteControllerForAnnotation)
- (void)showMenuForAnnotation:(PSPDFAnnotation *)annotation;

/// Shows a popover/modal controller to edit a PSPDFAnnotation.
- (void)showNoteControllerForAnnotation:(PSPDFAnnotation *)annotation;

// Helper to add a custom annotation to the view.
- (void)loadPageAnnotation:(PSPDFAnnotation *)annotation animated:(BOOL)animated;

@end


// Entends the UIScrollViewDelegate.
@interface PSPDFPageView (PSPDFScrollViewDelegateExtensions)

- (void)pspdf_scrollView:(UIScrollView *)scrollView willZoomToScale:(float)scale animated:(BOOL)animated;

@end

@interface PSPDFPageView (SubclassingHooks)

/**
    In PSPDFKit, annotations are managed in two ways:

    1) Annotations that are fixed and rendered with the page image.
    Those annotations are PSPDFHighlightAnnotation, PSPDFShapeAnnotation, PSPDFInkAnnotation and more.
    Pretty much all more or less "static" annotations are handled this way.
 
    2) Then, there are the more dynamic annotations like PSPDFLinkAnnotation and PSPDFNoteAnnotation.
    Those annotations are not part of the rendered image but are actual subviews in PSPDFPageView.
    
    Especially with PSPDFLinkAnnotation, the resulting views are - depending on the subtype - PSPDFVideoAnnotationView, PSPDFWebAnnotationView and much more. The classic PDF link is a PSPDFLinkAnnotationView.
 
    This method is called recursively with all annotation types except PSPDFAnnotationTypeLink | PSPDFAnnotationTypeNote.
*/
- (void)loadPageAnnotation:(PSPDFAnnotation *)annotation animated:(BOOL)animated;

/// Will be called automatically after kPSPDFInitialAnnotationLoadDelay.
/// Call manually to speed up rendering. Has no effect if called multiple times.
- (void)loadPageAnnotationsAnimated:(BOOL)animated;

@end

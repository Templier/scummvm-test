// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsd�rfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

/*
	BS_RenderObject
	---------------
	Dieses ist die Klasse die s�mtliche sichtbaren Objekte beschreibt. Alle anderen sichtbaren Objekte m�ssen von ihr abgeleitet werden.
	Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
	Alle BS_RenderObject Instanzen werden von einem BS_RenderObjectManager in einem Baum verwaltet.

	Autor: Malte Thiesen
*/

#ifndef SWORD25_RENDEROBJECT_H
#define SWORD25_RENDEROBJECT_H

// Includes
#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/renderobjectptr.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class BS_RenderObjectManager;
class BS_Bitmap;
class BS_Animation;
class BS_AnimationTemplate;
class BS_Panel;
class BS_Text;

// Klassendefinition
/**
	@brief 	Dieses ist die Klasse die s�mtliche sichtbaren Objekte beschreibt.
	
	Alle anderen sichtbaren Objekte m�ssen von ihr abgeleitet werden.
	Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
	Alle BS_RenderObject Instanzen werden von einem BS_RenderObjektManager in einem Baum verwaltet.
 */
class BS_RenderObject
{
public:
	// Konstanten
	// ----------
	enum TYPES
	{
		/// Das Wurzelobjekt. Siehe BS_RenderObjectManager
		TYPE_ROOT,
		/// Ein Image. Siehe BS_Bitmap.
		TYPE_STATICBITMAP,
		TYPE_DYNAMICBITMAP,
		/// Eine Animation. Siehe BS_Animation.
		TYPE_ANIMATION,
		/// Eine farbige Fl�che. Siehe BS_Panel.
		TYPE_PANEL,
		/// Ein Text. Siehe BS_Text.
		TYPE_TEXT,
		/// Ein unbekannter Objekttyp. Diesen Typ sollte kein Renderobjekt annehmen.
		TYPE_UNKNOWN
	};

	// Add-Methoden
	// ------------

	/**
	    @brief Erzeugt ein Bitmap als Kinderobjekt des Renderobjektes.
		@param FileName der Dateiname der Quellbilddatei
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Bitmap> AddBitmap(const std::string& FileName);
	/**
		@brief Erzeugt ein ver�nderbares Bitmap als Kinderobjekt des Renderobjektes.
		@param Width die Breite des Bitmaps
		@param Height die H�he des Bitmaps
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Bitmap> AddDynamicBitmap(unsigned int Width, unsigned int Height);
	/**
	    @brief Erzeugt eine Animation auf Basis einer Animationsdatei als Kinderobjekt des Renderobjektes.
		@param FileName der Dateiname der Quelldatei
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Animation> AddAnimation(const std::string& FileName);
	/**
		@brief Erzeugt eine Animation auf Basis eines Animationstemplate als Kinderobjekt des Renderobjektes.
		@param pAnimationTemplate ein Pointer auf das Animationstemplate
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
		@remark Das Renderobjekt �bernimmt die Verwaltung des Animationstemplate.
	*/
	BS_RenderObjectPtr<BS_Animation> AddAnimation(const BS_AnimationTemplate & AnimationTemplate);
	/**
	    @brief Erzeugt ein neues Farbpanel als Kinderobjekt des Renderobjektes.
		@param Width die Breite des Panels
		@param Height die H�he des Panels
		@param Color die Farbe des Panels.<br>
			   Der Standardwert ist Schwarz (BS_RGB(0, 0, 0)).
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	
	BS_RenderObjectPtr<BS_Panel> AddPanel(int Width, int Height, unsigned int Color = 0xff000000);
	/**
	    @brief Erzeugt ein Textobjekt als Kinderobjekt des Renderobjektes.
		@param Font der Dateiname des zu verwendenen Fonts
		@param Text der anzuzeigende Text.<br>
					Der Standardwert ist "".
		@return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
				Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Text> AddText(const std::string & Font, const std::string & Text = "");

	// Cast-Methoden
	// -------------
	/**
		@brief Castet das Objekt zu einem BS_Bitmap-Objekt wenn zul�ssig.
		@return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
				Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Bitmap> ToBitmap()
	{
		if (m_Type == TYPE_STATICBITMAP || m_Type == TYPE_DYNAMICBITMAP) return BS_RenderObjectPtr<BS_Bitmap>(this); else return BS_RenderObjectPtr<BS_Bitmap>();
	}
	/**
		@brief Castet das Objekt zu einem BS_Animation-Objekt wenn zul�ssig.
		@return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
				Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Animation> ToAnimation()
	{
		if (m_Type == TYPE_ANIMATION) return BS_RenderObjectPtr<BS_Animation>(this); else return BS_RenderObjectPtr<BS_Animation>();
	}
	/**
		@brief Castet das Objekt zu einem BS_Panel-Objekt wenn zul�ssig.
		@return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
				Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Panel> ToPanel()
	{
		if (m_Type == TYPE_PANEL) return BS_RenderObjectPtr<BS_Panel>(this); else return BS_RenderObjectPtr<BS_Panel>();
	}
	/**
		@brief Castet das Object zu einem BS_Text-Objekt wenn zul�ssig.
		@return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
				Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	BS_RenderObjectPtr<BS_Text> ToText()
	{
		if (m_Type == TYPE_TEXT) return BS_RenderObjectPtr<BS_Text>(this); else return BS_RenderObjectPtr<BS_Text>();
	}

	// Konstruktor / Desktruktor
	// -------------------------
	/**
	 	@brief Erzeugt ein neues BS_RenderObject.
		@param pKernel ein Pointer auf den Kernel
		@param pParent ein Pointer auf das Elternobjekt des neuen Objektes im Objektbaum.<br>
					   Der Pointer darf nicht NULL sein.
		@param Type der Objekttyp<br>
					Der Typ BS_RenderObject::TYPE_ROOT ist nicht zul�ssig. Wurzelknoten m�ssen mit dem alternativen Konstruktor erzeugt
					werden.
		@param Handle das Handle, welches dem Objekt zugewiesen werden soll.<br>
					Dieser Parameter erzwingt ein bestimmtes Handle f�r das neue Objekt, oder w�hlt automatisch ein Handle, wenn der Parameter 0 ist.
					Ist das gew�nschte Handle bereits vergeben, gibt GetInitSuccess() false zur�ck.<br>
					Der Standardwert ist 0.
		@remark Nach dem Aufruf des Konstruktors kann �ber die Methode GetInitSuccess() abgefragt werden, ob die Konstruktion erfolgreich war.<br>
				Es ist nicht notwendig alle BS_RenderObject Instanzen einzeln zu l�schen. Dieses geschiet automatisch beim L�schen eines
				Vorfahren oder beim L�schen des zust�ndigen BS_RenderObjectManager.
	 */
	BS_RenderObject(BS_RenderObjectPtr<BS_RenderObject> pParent, TYPES Type, unsigned int Handle = 0);
	virtual ~BS_RenderObject();

	// Interface
	// ---------
	/**
		@brief Rendert des Objekt und alle seine Unterobjekte.
		@return Gibt false zur�ck, falls beim Rendern ein Fehler aufgetreten ist.
		@remark Vor jedem Aufruf dieser Methode muss ein Aufruf von UpdateObjectState() erfolgt sein.
				Dieses kann entweder direkt geschehen oder durch den Aufruf von UpdateObjectState() an einem Vorfahren-Objekt.<br>
				Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	*/
	bool				Render();
	/**
	 	@brief Bereitet das Objekt und alle seine Unterobjekte auf einen Rendervorgang vor.
			   Hierbei werden alle Dirty-Rectangles berechnet und die Renderreihenfolge aktualisiert.
		@return Gibt false zur�ck, falls ein Fehler aufgetreten ist.
		@remark Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	 */
	bool				UpdateObjectState();
	/**
		@brief L�scht alle Kinderobjekte.
	*/
	void DeleteAllChildren();

	// Accessor-Methoden
	// -----------------
	/**
	 	@brief Setzt die Position des Objektes.
		@param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
		@param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetPos(int X, int Y);
	/**
	 	@brief Setzt die Position des Objektes auf der X-Achse.
		@param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetX(int X);
	/**
	 	@brief Setzt die Position des Objektes auf der Y-Achse.
		@param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetY(int Y);
	/**
	 	@brief Setzt den Z-Wert des Objektes.
		@param Z der neue Z-Wert des Objektes relativ zum Elternobjekt<br>
				 Negative Z-Werte sind nicht zul�ssig.
		@remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit h�herem
				Z-Wert gezeichnet. Je h�her der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
				Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
				Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	virtual void SetZ(int Z);
	/**
	 	@brief Setzt die Sichtbarkeit eine Objektes.
		@param Visible der neue Sichtbarkeits-Zustand des Objektes<br>
					   true entspricht sichtbar, false entspricht unsichtbar.
	 */
	virtual void SetVisible(bool Visible);
	/**
	 	@brief Gibt die Position des Objektes auf der X-Achse relativ zum Elternobjekt zur�ck.
	 */
	virtual int GetX() const { return m_X; }
	/**
	 	@brief Gibt die Position des Objektes auf der Y-Achse relativ zum Elternobjekt zur�ck.
	 */
	virtual int	GetY() const { return m_Y; }
	/**
		@brief Gibt die absolute Position des Objektes auf der X-Achse zur�ck.
	*/
	virtual int	GetAbsoluteX() const { return m_AbsoluteX; }
	/**
		@brief Gibt die absolute Position des Objektes auf der Y-Achse zur�ck.
	*/
	virtual int	GetAbsoluteY() const { return m_AbsoluteY; }
	/**
	 	@brief Gibt den Z-Wert des Objektes relativ zum Elternobjekt zur�ck.
		@remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit h�herem
				Z-Wert gezeichnet. Je h�her der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
				Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
				Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	int			GetZ() const { return m_Z; }
	/**
	 	@brief Gibt die Breite des Objektes zur�ck.
	 */
	int			GetWidth() const { return m_Width; }
	/**
	 	@brief Gibt die H�he des Objektes zur�ck.
	 */
	int			GetHeight() const { return m_Height; }
	/**
	 	@brief Gibt den Sichtbarkeitszustand des Objektes zur�ck.
		@return Gibt den Sichtbarkeitszustand des Objektes zur�ck.<br>
				true entspricht sichtbar, false entspricht unsichtbar.
	 */
	bool		IsVisible() const { return m_Visible; }
	/**
	 	@brief Gibt den Typ des Objektes zur�ck.
	 */
	TYPES		GetType() const { return m_Type; }
	/**
	 	@brief Gibt zur�ck, ob das Objekt erfolgreich initialisiert wurde.
		@remark H�sslicher Workaround um das Problem, dass Konstruktoren keine R�ckgabewerte haben.
	 */
	bool		GetInitSuccess() const { return m_InitSuccess; }
	/**
	 	@brief Gibt die Bounding-Box des Objektes zur�ck.
		@remark Diese Angabe erfolgt ausnahmsweise in Bildschirmkoordianten und nicht relativ zum Elternobjekt.
	 */
	const BS_Rect&	GetBBox() const { return m_BBox; }
	/**
		@brief Stellt sicher, dass das Objekt im n�chsten Frame neu gezeichnet wird.
	*/
	void ForceRefresh() { m_RefreshForced = true; };
	/**
		@brief Gibt das Handle des Objekte zur�ck.
	*/
	unsigned int GetHandle() const { return m_Handle; }

	// Persistenz-Methoden
	// -------------------
	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);
	// TODO: Evtl. protected
	bool PersistChildren(BS_OutputPersistenceBlock & Writer);
	bool UnpersistChildren(BS_InputPersistenceBlock & Reader);
	// TODO: Evtl. private
	BS_RenderObjectPtr<BS_RenderObject> RecreatePersistedRenderObject(BS_InputPersistenceBlock & Reader);

protected:
	// Typen
	// -----
	typedef std::vector<BS_RenderObjectPtr<BS_RenderObject> >			RENDEROBJECT_LIST;
	typedef std::vector<BS_RenderObjectPtr<BS_RenderObject> >::iterator	RENDEROBJECT_ITER;

	int			m_X;			///< Die X-Position des Objektes relativ zum Eltern-Objekt
	int			m_Y;			///< Die Y-Position des Objektes relativ zum Eltern-Objekt
	int			m_AbsoluteX;	///< Die absolute X-Position des Objektes
	int			m_AbsoluteY;	///< Die absolute Y-Position des Objektes
	int			m_Z;			///< Der Z-Wert des Objektes relativ zum Eltern-Objekt
	int			m_Width;		///< Die Breite des Objektes
	int			m_Height;		///< Die H�he des Objektes
	bool		m_Visible;		///< Ist true, wenn das Objekt sichtbar ist
	bool		m_ChildChanged;	///< Ist true, wenn sich ein Kinderobjekt ver�ndert hat
	TYPES		m_Type;			///< Der Objekttyp
	bool		m_InitSuccess;	///< Ist true, wenn Objekt erfolgreich intialisiert werden konnte
	BS_Rect		m_BBox;			///< Die Bounding-Box des Objektes in Bildschirmkoordinaten

	// Kopien der Variablen, die f�r die Errechnung des Dirty-Rects und zur Bestimmung der Objektver�nderung notwendig sind
	BS_Rect		m_OldBBox;
	int			m_OldX;
	int			m_OldY;
	int			m_OldZ;
	bool		m_OldVisible;

	/// Ein Pointer auf den BS_RenderObjektManager, der das Objekt verwaltet.
	BS_RenderObjectManager* m_ManagerPtr;

	// Render-Methode
	// --------------
	/**
	 	@brief Einschubmethode, die den tats�chlichen Redervorgang durchf�hrt.

		Diese Methode wird von Render() aufgerufen um das Objekt darzustellen.
		Diese Methode sollte von allen Klassen implementiert werden, die von BS_RederObject erben, um das Zeichnen umzusetzen.

		@return Gibt false zur�ck, falls das Rendern fehlgeschlagen ist.
		@remark 
	 */
	virtual bool DoRender() = 0; // { return true; };

	// RenderObject-Baum Variablen
	// ---------------------------
	// Der Baum legt die hierachische Ordnung der BS_RenderObjects fest.
	// Alle Eigenschaften wie X, Y, Z und Visible eines BS_RenderObjects sind relativ zu denen seines Vaters.
	// Au�erdem sind die Objekte von links nach rechts in ihrer Renderreihenfolge sortiert.
	// Das prim�re Sortierkriterium ist hierbei der Z-Wert und das sekund�re der Y-Wert (von oben nach unten).
	// Beispiel:
	//                  Screen
	//                 /  |   \.
	//               /    |     \.
	//             /      |       \.
	//           /        |         \.
	//     Background Interface   Maus
	//		/	\	   /  |   \.
	//    /      \    /   |    \.
	// George   T�r Icn1 Icn2 Icn3
	// 
	// Wenn jetzt das Interface mit SetVisible() ausgeblendet w�rde, verschwinden auch die Icons, die sich im Interface
	// befinden.
	// Wenn der Hintergrund bewegt wird (Scrolling), bewegen sich auch die darauf befindlichen Gegenst�nde und Personen.

	/// Ein Pointer auf das Elternobjekt.
	BS_RenderObjectPtr<BS_RenderObject>	m_ParentPtr;
	/// Die Liste der Kinderobjekte nach der Renderreihenfolge geordnet
	RENDEROBJECT_LIST			m_Children;

	/**
	 	@brief Gibt einen Pointer auf den BS_RenderObjektManager zur�ck, der das Objekt verwaltet.
	 */
	BS_RenderObjectManager*	GetManager() const { return m_ManagerPtr; }
	/**
		@brief F�gt dem Objekt ein neues Kinderobjekt hinzu.
		@param pObject ein Pointer auf das einzuf�gende Objekt
		@return Gibt false zur�ck, falls das Objekt nicht eingef�gt werden konnte.
	*/
	bool AddObject(BS_RenderObjectPtr<BS_RenderObject> pObject);

private:
	/// Ist true, wenn das Objekt in n�chsten Frame neu gezeichnet werden soll
	bool m_RefreshForced;

	unsigned int m_Handle;

	/**
	 	@brief Entfernt ein Objekt aus der Kinderliste.
		@param pObject ein Pointer auf das zu entfernende Objekt
		@return Gibt false zur�ck, falls das zu entfernende Objekt nicht in der Liste gefunden werden konnte.
	 */
	bool	DetatchChildren(BS_RenderObjectPtr<BS_RenderObject> pObject);
	/**
	 	@brief Berechnet die Bounding-Box und registriert das Dirty-Rect beim BS_RenderObjectManager.
	 */
	void	UpdateBoxes();
	/**
		@brief Berechnet die Bounding-Box des Objektes.
		@return Gibt die Bounding-Box des Objektes in Bildschirmkoordinaten zur�ck.
	*/
	BS_Rect CalcBoundingBox() const;
	/**
		@brief Berechnet das Dirty-Rectangle des Objektes.
		@return Gibt das Dirty-Rectangle des Objektes in Bildschirmkoordinaten zur�ck.
	*/
	BS_Rect CalcDirtyRect() const;
	/**
		@brief Berechnet die absolute Position des Objektes.
	*/
	void CalcAbsolutePos(int& X, int& Y) const;
	/**
		@brief Berechnet die absolute Position des Objektes auf der X-Achse.
	*/
	int CalcAbsoluteX() const;
	/**
		@brief Berechnet die absolute Position des Objektes.
	*/
	int CalcAbsoluteY() const;
	/**
	 	@brief Sortiert alle Kinderobjekte nach ihrem Renderang.
	 */
	void SortRenderObjects();
	/**
	 	@brief Validiert den Zustand eines Objektes nachdem die durch die Ver�nderung verursachten Folgen abgearbeitet wurden.
	 */
	void	ValidateObject();
	/**
		@brief Berechnet die absolute Position des Objektes und aller seiner Kinderobjekte neu.

		Diese Methode muss aufgerufen werden, wann immer sich die Position des Objektes ver�ndert. Damit die Kinderobjekte immer die
		richtige absolute Position haben.
	*/
	void UpdateAbsolutePos();
	/**
		@brief Teilt dem Objekt mit, dass sich eines seiner Kinderobjekte dahingehend ver�ndert hat, die eine erneute Bestimmung der
			   Rendereihenfolge verlangt.
	*/
	void	SignalChildChange() { m_ChildChanged = true; }
	/**
	 	@brief Berechnet des Schnittrechteck der Bounding-Box des Objektes mit einem anderen Objekt.
		@param pObjekt ein Pointer auf das Objekt mit dem geschnitten werden soll
		@param Result das Ergebnisrechteck
		@return Gibt false zur�ck, falls sich die Objekte gar nicht schneiden.
	 */
	bool	GetObjectIntersection(BS_RenderObjectPtr<BS_RenderObject> pObject, BS_Rect& Result);
	/**
		@brief Vergleichsoperator der auf Objektpointern basiert statt auf Objekten.
		@remark Diese Methode wird f�rs Sortieren der Kinderliste nach der Rendereihenfolge benutzt.
	*/
	static bool Greater(const BS_RenderObjectPtr<BS_RenderObject> lhs, const BS_RenderObjectPtr<BS_RenderObject> rhs);
};

#endif
<?php

use Nette\Application\UI\Form;
/*
 * Presenter pro vypis vsech nebo filtrovanych skol vcetne jejich fakult
 */
class SkolaPresenter extends BasePresenter
{
	private $skolaRepository;
	private $fakultaRepository;
	private $zamereniRepository;
	private $oblibeneRepository;
	private $filtr;
	private $recordSkola;
	private $recordZamereni;
	private $detailSkoly;
	private $detailZamereni;

	protected function startup()
	{
		parent::startup();
		$this->skolaRepository = $this->context->skolaRepository;
		$this->fakultaRepository = $this->context->fakultaRepository;
		$this->zamereniRepository = $this->context->zamereniRepository;
		$this->oblibeneRepository = $this->context->oblibeneRepository;
	}

	protected function createComponentFilterForm()
	{
		// Nacteni nazvu zamereni
		$zamereni = $this->zamereniRepository->getNamesAsArray();

		$form = new Form;
		$form->addSelect('filtr', 'Filtr podle zaměření: ', $zamereni)
			->setPrompt('Vše')->getControlPrototype()->onchange('submit();');

		// Nastaveni vybrane hodnoty
		$form->setDefaults(array('filtr' => $this->filtr));
		$form->setMethod('GET');

		return $form;
	}

	public function actionDetail($id) {
		$this->detailSkoly = $this->skolaRepository->findBy(array('Nazev_skoly' => $id))->fetch();
		if ($this->detailSkoly === FALSE) {
			$this->setView('notFound');
		}
	}

	public function actionAddskola() {
		if (!$this->getUser()->isInRole('admin')) {
			$this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
			$this->redirect('Skola:');
		}
	}

	public function actionEditskola($nazev_skoly) {
		if (!$this->getUser()->isInRole('admin')) {
			$this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
			$this->redirect('Skola:');
		}
		$this->detailSkoly = $this->skolaRepository->findBy(array('Nazev_skoly' => $nazev_skoly))->fetch();
		if ($this->detailSkoly === FALSE) {
			$this->setView('notFound');
		}
		$this->recordSkola = $this->skolaRepository->findBy(array('Nazev_skoly' => $nazev_skoly))->fetch();
		if (!$this->recordSkola) {
			$this->setView('notFound');
		}
		$this['editSchoolForm']->setDefaults(array(
			//'nazev_skoly' => $this->recordSkola['Nazev_skoly'],
			'adresa_skoly' => $this->recordSkola['Adresa_skoly'],
			'email_skoly' => $this->recordSkola['Email_skoly'],
			'web_skoly' => $this->recordSkola['Web_skoly'],
			'rektor' => $this->recordSkola['Rektor'],
			'zamereni' => $this->recordSkola['Nazev_zamereni']
		));
	}

	public function actionAddzamereni() {
		if (!$this->getUser()->isInRole('admin')) {
			$this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
			$this->redirect('Skola:');
		}
	}

	public function actionEditzamereni($nazev_zamereni) {
		if (!$this->getUser()->isInRole('admin')) {
			$this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
			$this->redirect('Skola:');
		}
		$this->recordZamereni = $this->zamereniRepository->findBy(array('Nazev_zamereni' => $nazev_zamereni))->fetch();
		if (!$this->recordZamereni) {
			$this->setView('notFound');
		}
		$this->detailZamereni = $this->zamereniRepository->findBy(array('Nazev_zamereni' => $nazev_zamereni))->fetch();
		if ($this->detailZamereni === FALSE) {
			$this->setView('notFound');
		}
		$this['editZamereniForm']->setDefaults(array(
			'popis_zamereni' => $this->recordZamereni['Popis_zamereni'],
			'uplatneni' => $this->recordZamereni['Uplatneni']
		));
	}

	public function renderDetail()
	{
		$this->template->skola = $this->detailSkoly;
		$this->template->fakulty = $this->fakultaRepository->findBy(array('Nazev_skoly' => $this->detailSkoly->Nazev_skoly));

		$fav = $this->oblibeneRepository->findBy(array('User_id' => $this->getUser()->getId(), 'Nazev_skoly' => $this->detailSkoly->Nazev_skoly))->fetch();
		if ($fav != false)
			$this->template->favId = $fav->ID_oblibene;
		else
			$this->template->favId = 0;

		//$this->redirect($this);
	}

	public function renderDefault($filtr = null)
	{
		// Ulozeni aktualniho stavu filtru
		$this->filtr = $filtr;

		if ($filtr == null)
			$this->template->skoly = $this->skolaRepository->findAll()->order('Nazev_skoly', 'ASC');
		else
			$this->template->skoly = $this->skolaRepository->findBy(array('Nazev_zamereni' => $filtr))->order('Nazev_skoly', 'ASC');

		$this->template->fakulty = $this->fakultaRepository->showFacultiesNames();
		//$this->redirect($this);
	}

	public function renderEditskola() 
	{
		$this->template->nazev_skoly = $this->detailSkoly->Nazev_skoly;
	}

	public function renderEditzamereni() 
	{
		$this->template->nazev_zamereni = $this->detailZamereni->Nazev_zamereni;
	}

	/*
	 * pridani zamereni
	 */
	protected function createComponentAddZamereniForm() {
		$form = new Form;
		$form->addText('nazev_zamereni', 'Název zaměření:')
			->setRequired('Zadajte prosím název zaměření');
		$form->addTextArea('popis_zamereni', 'Stručný popis zaměření:')
			->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 80 znaku', 80);
		$form->addTextArea('uplatneni', 'Uplatnění:')
			->addRule(Form::MAX_LENGTH, 'Maximální délka uplatnění je 50 znaku', 50);
		$form->addSubmit('addZamereni', 'Přidat')
			->onClick[] = $this->addZamereniFormSubmitted;
		$form->addSubmit('cancel', 'Storno')
			->setValidationScope(FALSE)
			->onClick[] = callback($this, 'zamereniFormCancel');
		return $form;
	}

	public function addZamereniFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
		$values = $btn->form->getValues();
		if ( $this->zamereniRepository->findByName($values->nazev_zamereni)) {
			$btn->form->addError('Toto zaměření již existuje');
			return;
		}
		try {
			$this->zamereniRepository->addZamereni($values);
			$this->flashMessage('Operace byla úspěšně provedena', 'success');
			$this->redirect('Skola:default');
		}
		catch (Exception $e) {
			if (!($e instanceof Nette\Application\AbortException)) {
				$this->flashMessage('Operaci nebylo možné provést', 'error');
			}
			$this->redirect('Skola:default');
		}
	}

	public function zamereniFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
		$this->redirect('Skola:');
	}

	/*
	 * editace zamereni
	 */
	protected function createComponentEditZamereniForm() {
		$form = new Form;
		$form->addTextArea('popis_zamereni', 'Stručný popis zaměření:')
			->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 80 znaku', 80);
		$form->addTextArea('uplatneni', 'Uplatnění:')
			->addRule(Form::MAX_LENGTH, 'Maximální délka uplatnění je 50 znaku', 50);
		$form->addSubmit('addZamereni', 'Potvrdit změny')
			->onClick[] = $this->editZamereniFormSubmitted;
		$form->addSubmit('cancel', 'Storno')
			->setValidationScope(FALSE)
			->onClick[] = callback($this, 'zamereniFormCancel');
		return $form;
	}

	public function editZamereniFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
		$values = $btn->form->getValues();
		try {
			$this->zamereniRepository->getConnection()->exec(
				'UPDATE zamereni SET Popis_zamereni=?, Uplatneni=? WHERE Nazev_zamereni=?',
				$values->popis_zamereni, $values->uplatneni, $this->recordZamereni['Nazev_zamereni']);
			$this->flashMessage('Operace byla úspěšně provedena', 'success');
			$this->redirect('Skola:default');
		}
		catch (Exception $e) {
			if (!($e instanceof Nette\Application\AbortException)) {
				$this->flashMessage('Operaci nebylo možné provést', 'error');
			}
			$this->redirect('Skola:default');
		}
	}

	/*
	 * pridani skoly
	 */
	protected function createComponentAddSchoolForm() {
		$zamereni = $this->zamereniRepository->getNamesAsArray();
		$form = new Form;
		$form->addText('nazev_skoly', 'Název školy:')
			->setRequired('Musíte zadat název školy');
		$form->addText('adresa_skoly', 'Adresa školy:')
			->setRequired('Musíte zadat adresu školy');
		$form->addText('email_skoly', 'Emailová adresa:');
		$form->addText('web_skoly', 'Webové stránky:');
		$form->addText('rektor', 'Jméno rektora:')
			->setRequired('Musíte zadat jméno rektora:');
		$form->addSelect('zamereni', 'Zaměření:', $zamereni)
			->setPrompt('Zvolte zaměření')
			->setRequired('Vyberte prosím zaměření školy');
		$form->addUpload('logo_skoly', 'Logo školy:')
			->addCondition($form::FILLED)
			->addRule($form::MIME_TYPE, 'Obrázek musí být ve formátu jpeg', 'image/jpeg')
			->addRule($form::MAX_FILE_SIZE, 'Maximální velikost souboru je 100 KiB', 100*1024);
		$form->addSubmit('addSchool', 'Přidat')
			->onClick[] = $this->addSchoolFormSubmitted;
		$form->addSubmit('cancel', 'Storno')
			->setValidationScope(FALSE)
			->onClick[] = callback($this, 'schoolFormCancel');
		return $form;
	}

	public function addSchoolFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
		$values = $btn->form->getValues();
		$row = $this->skolaRepository->findByName($values->nazev_skoly);
		if ($row) {
			$btn->form->addError('Škola s timto jmenem již existuje.');
			return;
		}
		try {
			$this->skolaRepository->addSkola($values);
			$this->flashMessage('Operace byla úspěšně provedena', 'success');
			$this->redirect('Skola:default');
		}
		catch (Exception $e) {
			if (!($e instanceof Nette\Application\AbortException)) {
				$this->flashMessage('Operaci nebylo možné provést', 'error');
			}
			else {
				$this->redirect('Skola:detail', $values->nazev_skoly);
			}
			$this->redirect('Skola:default');
		}
	}

	public function schoolFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
		$this->redirect('Skola:');
	}

	/*
	 * editace detailu skoly
	 */
	protected function createComponentEditSchoolForm() 
	{
		$zamereni = $this->zamereniRepository->getNamesAsArray();
		$form = new Form;
		//$form->addText('nazev_skoly', 'Název školy:')
		//    ->setRequired('Zadejte prosím název školy');
		$form->addText('adresa_skoly', 'Adresa školy:')
			->setRequired('Musíte zadat adresu školy');
		$form->addText('email_skoly', 'Emailová adresa:');
		$form->addText('web_skoly', 'Webové stránky:');
		$form->addText('rektor', 'Jméno rektora:')
			->setRequired('Musíte zadat jméno rektora:');
		$form->addSelect('zamereni', 'Zaměření:', $zamereni)
			->setPrompt('Zvolte zaměření')
			->setRequired('Vyberte prosím zaměření školy');
		$form->addUpload('logo_skoly', 'Logo školy:')
			->addCondition($form::FILLED)
			->addRule($form::MIME_TYPE, 'Obrázek musí být ve formátu jpeg', 'image/jpeg')
			->addRule($form::MAX_FILE_SIZE, 'Maximální velikost souboru je 100 KiB', 100*1024);
		$form->addSubmit('addSchool', 'Potvrdit změny:')
			->onClick[] = $this->editSchoolFormSubmitted;
		$form->addSubmit('cancel', 'Storno')
			->setValidationScope(FALSE)
			->onClick[] = callback($this, 'schoolFormCancel');
		return $form;
	}

	public function editSchoolFormSubmitted(Nette\Forms\Controls\SubmitButton $btn)
	{
		$values = $btn->form->getValues();
		try {
			$this->skolaRepository->getConnection()->exec(
				'UPDATE skola SET Adresa_skoly=?, Email_skoly=?, Web_skoly=?, Rektor=?, Nazev_zamereni=?
				WHERE Nazev_skoly=?', $values->adresa_skoly, $values->email_skoly, $values->web_skoly,
				$values->rektor, $values->zamereni, $this->recordSkola['Nazev_skoly']);
			if ($values->logo_skoly != '') {
				$logo = $values->logo_skoly->toImage();
				$logo->save(__DIR__.'/../../www/images/logos/'.str_replace(' ', '_',$this->recordSkola['Nazev_skoly']).'.jpg', 98);
			}
			$this->flashMessage('Operace byla úspěšně provedena', 'success');
			$this->redirect('Skola:default', $this->recordSkola['Nazev_skoly']);
		}
		catch (Exception $e) {
			if (!($e instanceof Nette\Application\AbortException)) {
				$this->flashMessage('Operaci nebylo možné provést', 'error');
			}
			$this->redirect('Skola:detail', $this->recordSkola['Nazev_skoly']);
		}
	}

	public function deleteSkola($nazev)
	{
		$this->skolaRepository->deleteSkola($nazev);
		$this->flashMessage('Škola byla úspěšně odstraněna.', 'success');
		$this->redirect('Skola:');
	}

	public function deleteOblibene($id) {
		$this->oblibeneRepository->deleteOblibene($id);
		$this->flashMessage('Položka byla odstraněna ze seznamu oblíbených', 'success');
	}

	public function deleteZamereni($nazev_zamereni) {
		$this->zamereniRepository->deleteZamereni($nazev_zamereni);
		$this->flashMessage('Zaměření bylo odstraněno', 'success');
	}

	/**
	 *	Fuknce pro zachyceni signalu pro pridani polozky do oblibenych 
	 */
	public function handleAddFav() {
		if (!$this->oblibeneRepository->findAll()
			->where('User_id', $this->getUser()->getId())
			->where('Nazev_skoly', $this->detailSkoly->Nazev_skoly)->fetch()) {
				$values = array('ID_oblibene' => NULL,
					'User_id' => $this->getUser()->getId(),
					'Nazev_skoly' => $this->detailSkoly->Nazev_skoly,
					'ID_fakulty' => NULL);
				$this->oblibeneRepository->addFav($values);
			}
		$this->flashMessage('Škola byla přidána do oblíbených.', 'success');
	}
}
